#include <iostream>
#include <string>
#include <vector>
#include <algorithm>

using namespace std;

enum EDirection
{
    eRight,
    eLeft,
    eDown,
    eUp
};

struct Coord
{
   int X;
   int Y;
};

struct WallDatas
{
   Coord    Position;
   string   Orientation;

   const string ToString (void) const
   {
      return to_string(Position.X) + " " + to_string(Position.Y) + " " + Orientation;
   }
};

#define POIDS_MUR               9999
#define COEF_POIDS_CHEMIN       2

class CPlayerDatas
{
public:
   typedef vector<CPlayerDatas> Vector;
public:
   CPlayerDatas (const EDirection& aDirection,const Coord& aPosition, const int& aWallsLeft):
      mDirection (aDirection),
      mPosition  (aPosition),
      mWallsLeft (aWallsLeft),
      mbPlay     (true)
   {};
   ~CPlayerDatas (void) {};

   inline const EDirection GetDirection (void) const
   {
      return mDirection;
   };

   inline const Coord GetPosition (void) const
   {
      return mPosition;
   };

   inline const int GetNbWallsLeft (void) const
   {
      return mWallsLeft;
   };

   inline const vector<int>& GetPCC (void) const
   {
      return mPCC;
   };

   inline void SetPCC (const vector<int>& aPCC)
   {
      mPCC = aPCC;
   };

   inline bool Play (void) const
   {
      return mbPlay;
   };

   inline void SetOut (void)
   {
      mbPlay = false;
   };

private:
   const EDirection  mDirection;
   const Coord       mPosition;
   const int         mWallsLeft;

   bool              mbPlay;
   vector<int>       mPCC;
};

/**
 * Classe de calcul du plus court chemin
 */
class CIA
{
public:
    CIA (const int aWidth, const int aHeight);
    ~CIA(void);
    
    void    ConstruireMatriceGraphe       (void);
    void    AjoutMurMatriceGraphe         (const WallDatas& aWallDatas);
    void    AjoutMurMatriceGrapheLite     (const WallDatas& aWallDatas, const bool abDestroy);
    bool    CalculPlusCourtCheminPlayer   (CPlayerDatas& aPlayersDatas);
    void    CalculCheminMinimaux          (void);
    
    string  GetNextDirection            (const vector<int>& aPlusCourtChemin);
    string  BuildWallInFrontOfPlayer    (CPlayerDatas::Vector& aPlayersDatas, const int& aIdPlayer, const vector<WallDatas>& aWallsBuilt);

private:
    bool CalculPlusCourtChemin  (const int aNumCaseDepart, const int aNumCaseArrivee, vector<int>& aOutPlusCourtChemin);
    
    bool BuildVerticalWall    (CPlayerDatas::Vector& aPlayersDatas, const int& aIdPlayer, const vector<WallDatas>& aWallsBuilt, const Coord& aCoordWall1, const Coord& aCoordWall2, WallDatas& aWallDatas, unsigned int& aLengthPCC);
    bool BuildHorizontalWall  (CPlayerDatas::Vector& aPlayersDatas, const int& aIdPlayer, const vector<WallDatas>& aWallsBuilt, const Coord& aCoordWall1, const Coord& aCoordWall2, WallDatas& aWallDatas, unsigned int& aLengthPCC);

    bool ChercheNewPCCPlayer (CPlayerDatas::Vector& aPlayersDatas, const int& aIdPlayer, const WallDatas& aWallDatas, unsigned int& aLengthPCC);
    void SelectionneBestWall (const bool& abConstructible1, const WallDatas& aWallDatas1, const unsigned int& aLengthPCC1, const bool& abConstructible2, const WallDatas& aWallDatas2, const unsigned int& aLengthPCC2, WallDatas& aBestWallDatas, unsigned int& aLengthBestPCC);

    bool IsConstructibleVertical    (const vector<WallDatas>& aWallsBuilt, const Coord aNewWall) const;
    bool IsConstructibleHorizontal  (const vector<WallDatas>& aWallsBuilt, const Coord aNewWall) const;
    
    bool IsCheminPossiblePlayer (const CPlayerDatas& aPlayersDatas) const;
    bool IsCheminPossible       (const int aNumCaseDepart, const int aNumCaseArrivee) const;
private:
    const int mWidth;
    const int mHeight;
    
    // Différentes matrices de calcul de plus court chemin
    int **mMatriceGraph;
    int **mMatriceGraphCalcul;
    int **mCheminsMinimaux;
};

CIA::CIA (const int aWidth, const int aHeight):
    mWidth  (aWidth),
    mHeight (aHeight)
{
    const int NbLigneColonne = mWidth * mHeight;
    
    // Allocation dynamique de la matrice du graph et des celle des chemins minimaux
    mMatriceGraph = new int* [NbLigneColonne];
    mMatriceGraphCalcul = new int* [NbLigneColonne];
    mCheminsMinimaux = new int* [NbLigneColonne];
    for (int IterLigne = 0; IterLigne < NbLigneColonne ; IterLigne++)
    {
        mMatriceGraph[IterLigne] = new int[NbLigneColonne];
        mMatriceGraphCalcul[IterLigne] = new int[NbLigneColonne];
        mCheminsMinimaux[IterLigne] = new int[NbLigneColonne];
    }
    
    // Mise à zéro de la matrice du graph
    for (int IterLigne= 0; IterLigne < NbLigneColonne; ++IterLigne)
    {
        for (int IterColonne= 0; IterColonne < NbLigneColonne; ++IterColonne)
        {
            if (IterLigne == IterColonne)
            {
                mMatriceGraph[IterColonne][IterLigne] = 0;
            }
            else
            {
                mMatriceGraph[IterColonne][IterLigne] = POIDS_MUR;
            }
        }
    }
}

CIA::~CIA (void)
{
    const int NbLigneColonne = mWidth * mHeight;
    for(int IterLigne = 0 ; IterLigne < NbLigneColonne ; ++IterLigne)
    {
        delete[] mMatriceGraph[IterLigne];
        delete[] mCheminsMinimaux[IterLigne];
    }
    delete[] mMatriceGraph;
    delete[] mCheminsMinimaux;
}

void CIA::ConstruireMatriceGraphe (void)
{
    int IterLargeur = 0;
    int IterHauteur = 0;
    int NumCaseCourante = -1;
    int NbCaseLargeur = mWidth;
    int NbCaseHauteur = mHeight;
    
    // Parcours du plateau pour construire la matrice
    for (; IterLargeur < NbCaseLargeur; ++IterLargeur)
    {
        for (IterHauteur = 0; IterHauteur < NbCaseHauteur; ++IterHauteur)
        {
            NumCaseCourante = IterLargeur + (IterHauteur * NbCaseLargeur);
        
            // On observe les cases autour
            // Si on est pas sur la bordure gauche,
            if (IterLargeur != 0)
            {
                int PoidsCheminGauche = 1;
                int PoidsCheminDroite = 1;
                // On est sur la bordure du haut ou du bas
                if ((IterHauteur == 0) || (IterHauteur == (NbCaseHauteur - 1)))
                {
                    PoidsCheminGauche = (COEF_POIDS_CHEMIN * COEF_POIDS_CHEMIN);
                    PoidsCheminDroite = (COEF_POIDS_CHEMIN * COEF_POIDS_CHEMIN);
                }
                else 
                {
                    // On est sur seconde ligne ou l'avant dernière
                    if ((IterHauteur == 1) || (IterHauteur == (NbCaseHauteur - 2)))
                    {
                        //PoidsCheminGauche = COEF_POIDS_CHEMIN;
                        //PoidsCheminDroite = COEF_POIDS_CHEMIN;
                        PoidsCheminGauche = 1;
                        PoidsCheminDroite = 1;
                    }
                    //On est sur la seconde colonne
                    if (IterLargeur == 1)
                    {
                        //PoidsCheminGauche = (COEF_POIDS_CHEMIN * COEF_POIDS_CHEMIN);
                        PoidsCheminGauche = COEF_POIDS_CHEMIN;
                    }
                    //On est sur l'avant dernière colonne
                    else if (IterLargeur == (NbCaseLargeur - 2))
                    {
                        //PoidsCheminDroite = (COEF_POIDS_CHEMIN * COEF_POIDS_CHEMIN);
                        PoidsCheminDroite = COEF_POIDS_CHEMIN;
                    }
                }

                mMatriceGraph[NumCaseCourante][(IterLargeur - 1) + IterHauteur * NbCaseLargeur] = PoidsCheminGauche;
                mMatriceGraph[(IterLargeur - 1) + IterHauteur * NbCaseLargeur][NumCaseCourante] = PoidsCheminDroite;
            }
            // Si on est pas sur la bordure droite,
            if (IterLargeur != (NbCaseLargeur - 1))
            {
                int PoidsCheminGauche = 1;
                int PoidsCheminDroite = 1;
                // On est sur la bordure du haut ou du bas
                if ((IterHauteur == 0) || (IterHauteur == (NbCaseHauteur - 1)))
                {
                    PoidsCheminGauche = (COEF_POIDS_CHEMIN * COEF_POIDS_CHEMIN);
                    PoidsCheminDroite = (COEF_POIDS_CHEMIN * COEF_POIDS_CHEMIN);
                }
                else 
                {
                    // On est sur seconde ligne ou l'avant dernière
                    if ((IterHauteur == 1) || (IterHauteur == (NbCaseHauteur - 2)))
                    {
                        //PoidsCheminGauche = COEF_POIDS_CHEMIN;
                        //PoidsCheminDroite = COEF_POIDS_CHEMIN;
                        PoidsCheminGauche = 1;
                        PoidsCheminDroite = 1;
                    }
                    //On est sur la seconde colonne
                    if (IterLargeur == 1)
                    {
                        //PoidsCheminGauche = (COEF_POIDS_CHEMIN * COEF_POIDS_CHEMIN);
                        PoidsCheminGauche = COEF_POIDS_CHEMIN;
                    }
                    //On est sur l'avant dernière colonne
                    else if (IterLargeur == (NbCaseLargeur - 2))
                    {
                        //PoidsCheminDroite = (COEF_POIDS_CHEMIN * COEF_POIDS_CHEMIN);
                        PoidsCheminDroite = COEF_POIDS_CHEMIN;
                    }
                }
                mMatriceGraph[NumCaseCourante][(IterLargeur + 1) + IterHauteur * NbCaseLargeur] = PoidsCheminDroite;
                mMatriceGraph[(IterLargeur + 1) + IterHauteur * NbCaseLargeur][NumCaseCourante] = PoidsCheminGauche;
            }
            // Si on est pas sur la bordure du haut,
            if (IterHauteur != 0)
            {
                int PoidsCheminHaut = 1;
                int PoidsCheminBas = 1;
                // On est sur la bordure de gauche ou de droite
                if ((IterLargeur == 0) || (IterLargeur == (NbCaseLargeur - 1)))
                {
                    PoidsCheminHaut = (COEF_POIDS_CHEMIN * COEF_POIDS_CHEMIN);
                    PoidsCheminBas = (COEF_POIDS_CHEMIN * COEF_POIDS_CHEMIN);
                }
                else 
                {
                    // On est sur seconde colonne ou l'avant dernière
                    if ((IterLargeur == 1) || (IterLargeur == (NbCaseLargeur - 2)))
                    {
                        //PoidsCheminHaut = COEF_POIDS_CHEMIN;
                        //PoidsCheminBas = COEF_POIDS_CHEMIN;
                        PoidsCheminHaut = 1;
                        PoidsCheminBas = 1;
                    }
                    //On est sur la seconde ligne
                    if (IterHauteur == 1)
                    {
                        //PoidsCheminHaut = (COEF_POIDS_CHEMIN * COEF_POIDS_CHEMIN);
                        PoidsCheminHaut = COEF_POIDS_CHEMIN;
                    }
                    //On est sur l'avant dernière ligne
                    else if (IterHauteur == (NbCaseHauteur - 2))
                    {
                        //PoidsCheminBas = (COEF_POIDS_CHEMIN * COEF_POIDS_CHEMIN);
                        PoidsCheminBas = COEF_POIDS_CHEMIN;
                    }
                }
                mMatriceGraph[NumCaseCourante][IterLargeur + (IterHauteur - 1) * NbCaseLargeur] = PoidsCheminHaut;
                mMatriceGraph[IterLargeur + (IterHauteur - 1) * NbCaseLargeur][NumCaseCourante] = PoidsCheminBas;
            }
            // Si on est pas sur la bordure du bas,
            if (IterHauteur != (NbCaseHauteur - 1))
            {
                int PoidsCheminHaut = 1;
                int PoidsCheminBas = 1;
                // On est sur la bordure de gauche ou de droite
                if ((IterLargeur == 0) || (IterLargeur == (NbCaseLargeur - 1)))
                {
                    PoidsCheminHaut = (COEF_POIDS_CHEMIN * COEF_POIDS_CHEMIN);
                    PoidsCheminBas  = (COEF_POIDS_CHEMIN * COEF_POIDS_CHEMIN);
                }
                else 
                {
                    // On est sur seconde colonne ou l'avant dernière
                    if ((IterLargeur == 1) || (IterLargeur == (NbCaseLargeur - 2)))
                    {
                        //PoidsCheminHaut = COEF_POIDS_CHEMIN;
                        //PoidsCheminBas = COEF_POIDS_CHEMIN;
                        PoidsCheminHaut = 1;
                        PoidsCheminBas = 1;
                    }
                    //On est sur la seconde ligne
                    if (IterHauteur == 1)
                    {
                        //PoidsCheminHaut = (COEF_POIDS_CHEMIN * COEF_POIDS_CHEMIN);
                        PoidsCheminHaut = COEF_POIDS_CHEMIN;
                    }
                    //On est sur l'avant dernière ligne
                    else if (IterHauteur == (NbCaseHauteur - 2))
                    {
                        //PoidsCheminBas = (COEF_POIDS_CHEMIN * COEF_POIDS_CHEMIN);
                        PoidsCheminBas = COEF_POIDS_CHEMIN;
                    }
                }
                mMatriceGraph[NumCaseCourante][IterLargeur + (IterHauteur + 1) * NbCaseLargeur] = PoidsCheminBas;
                mMatriceGraph[IterLargeur + (IterHauteur + 1) * NbCaseLargeur][NumCaseCourante] = PoidsCheminHaut;
            }
        }
    }
}

void CIA::CalculCheminMinimaux (void)
{
    for (int IterLigne = 0; IterLigne < mWidth * mHeight; ++IterLigne)
    {
        for (int IterColonne = 0; IterColonne < mWidth * mHeight; ++IterColonne)
        {
            mMatriceGraphCalcul[IterLigne][IterColonne] = mMatriceGraph[IterLigne][IterColonne];
            if ((IterLigne != IterColonne) && (mMatriceGraph[IterLigne][IterColonne] < POIDS_MUR))
            {
                mCheminsMinimaux[IterLigne][IterColonne] = IterLigne;
            }
            else
            {
                mCheminsMinimaux[IterLigne][IterColonne] = POIDS_MUR;
            }
        }
    }
    for (int IterInterm = 0; IterInterm < mWidth * mHeight; ++IterInterm)
    {
        for (int IterLigne = 0; IterLigne < mWidth * mHeight; ++IterLigne)
        {
            if (mMatriceGraphCalcul[IterLigne][IterInterm] < POIDS_MUR)
            {
                for (int IterColonne = 0; IterColonne < mWidth * mHeight; ++IterColonne)
                {
                    int Cheminik = mMatriceGraphCalcul[IterLigne][IterInterm];
                    int Cheminkj = mMatriceGraphCalcul[IterInterm][IterColonne];
                    if ((Cheminik < POIDS_MUR) && (Cheminkj < POIDS_MUR))
                    {
                        int Cheminij = Cheminik + Cheminkj;
                        if (Cheminij < mMatriceGraphCalcul[IterLigne][IterColonne])
                        {
                            mMatriceGraphCalcul[IterLigne][IterColonne] = Cheminij;
                            mCheminsMinimaux[IterLigne][IterColonne] = mCheminsMinimaux[IterInterm][IterColonne];
                        }
                    }
                }
            }
        }
    }
}

bool CIA::CalculPlusCourtCheminPlayer (CPlayerDatas& aPlayersDatas)
{
   bool bReturn = false;
   if ((aPlayersDatas.GetPosition ().X == -1) || (aPlayersDatas.GetPosition ().Y == -1) || (aPlayersDatas.GetDirection () == -1))
   {
      aPlayersDatas.SetOut ();
      bReturn = true;
   }
   else
   {
      vector<int> CasesArrivees;
      switch (aPlayersDatas.GetDirection ())
      {
         case eRight:
            for (int i=0; i<mHeight; ++i)
               CasesArrivees.push_back ((mWidth - 1) + i * mWidth);
            break;
         case eLeft:
            for (int i=0; i<mHeight; ++i)
               CasesArrivees.push_back (i * mWidth);
            break;
         case eDown:
            for (int i=0; i<mWidth; ++i)
               CasesArrivees.push_back (i + (mHeight - 1) * mWidth);
            break;
         case eUp:
         default:
            break;
      }
      unsigned int DistanceMin = 99;
      for (unsigned int i=0; i < CasesArrivees.size();++i)
      {
         vector<int> PCC;
         bReturn = CalculPlusCourtChemin (aPlayersDatas.GetPosition ().X + aPlayersDatas.GetPosition ().Y * mWidth, CasesArrivees[i], PCC);
         if (bReturn && (1 < PCC.size()) && (PCC.size() < DistanceMin))
         {
            DistanceMin = PCC.size();
            aPlayersDatas.SetPCC (PCC);
         }
      }
   }
   return bReturn;
}

bool CIA::CalculPlusCourtChemin (const int aNumCaseDepart, const int aNumCaseArrivee, vector<int>& aOutPlusCourtChemin)
{
    aOutPlusCourtChemin.clear();
    
    // Récupération du plus court chemin
    int NumCaseCourante = aNumCaseDepart;
    int NbIterMax = mHeight * mWidth;
    int NbIter = 0;
    aOutPlusCourtChemin.push_back (NumCaseCourante);
    while ((NbIter != NbIterMax) && (NumCaseCourante != aNumCaseArrivee) && (NumCaseCourante < POIDS_MUR))
    {
        NumCaseCourante = mCheminsMinimaux[aNumCaseArrivee][NumCaseCourante];
        aOutPlusCourtChemin.push_back (NumCaseCourante);
        ++NbIter;
    }

//    cerr << "[" << aNumCaseDepart << ", " << aNumCaseArrivee << "]" << " => " << aOutPlusCourtChemin.size () << endl;
//    vector <int>::const_iterator IterPlusCourtChemin = aOutPlusCourtChemin.begin ();
//    for (; IterPlusCourtChemin != aOutPlusCourtChemin.end (); ++IterPlusCourtChemin)
//  {
//        cerr << (*IterPlusCourtChemin) << " ";
//    }
//    cerr << endl;
    
    return ((NbIter != NbIterMax) && (NumCaseCourante < POIDS_MUR));
}

string CIA::GetNextDirection (const vector<int>& aPlusCourtChemin)
{
    // LEFT, RIGHT, UP, DOWN
    string Direction = "RIGHT";
    
    // Si il y a une prochaine case
    if (aPlusCourtChemin.size () >= 2)
    {
        // Si la prochaine case est celle de gauche
        if (aPlusCourtChemin[1] == (aPlusCourtChemin[0] - 1))
        {
            Direction = "LEFT";
        }
        // Si la prochaine case est celle de droite
        else if (aPlusCourtChemin[1] == (aPlusCourtChemin[0] + 1))
        {
            Direction = "RIGHT";
        }
        // Si la prochaine case est celle du haut
        else if (aPlusCourtChemin[1] == (aPlusCourtChemin[0] - mWidth))
        {
            Direction = "UP";
        }
        // Si la prochaine case est celle du bas
        else if (aPlusCourtChemin[1] == (aPlusCourtChemin[0] + mWidth))
        {
            Direction = "DOWN";
        }
        else
        {
            // Cas d'erreur car impossible
        }
    }
    return Direction;
}

string CIA::BuildWallInFrontOfPlayer (CPlayerDatas::Vector& aPlayersDatas, const int& aIdPlayer, const vector<WallDatas>& aWallsBuilt)
{
   const vector<int> PCCPlayer = aPlayersDatas[aIdPlayer].GetPCC ();

   string         WallBuilding;
   WallDatas      BestWallDatas;
   unsigned int   LengthBestWall = 0;

   for (unsigned int iCase = 0; iCase < (PCCPlayer.size () - 1); ++iCase)
   {
      const unsigned int iCaseNext = iCase + 1;

      bool bConstructible = false;
      WallDatas      WallDatas;
      unsigned int   LengthPCC;
      // Si la prochaine case est celle de gauche
      if (PCCPlayer[iCaseNext] == (PCCPlayer[iCase] - 1))
      {
         Coord CoordNewWall1 = {PCCPlayer[iCase] % mWidth, PCCPlayer[iCase] / mWidth};
         Coord CoordNewWall2 = {CoordNewWall1.X, CoordNewWall1.Y - 1};

         bConstructible = BuildVerticalWall (aPlayersDatas, aIdPlayer, aWallsBuilt, CoordNewWall1, CoordNewWall2, WallDatas, LengthPCC);
      }
      // Si la prochaine case est celle de droite
      else if (PCCPlayer[iCaseNext] == (PCCPlayer[iCase] + 1))
      {
         Coord CoordNewWall1 = {PCCPlayer[iCase] % mWidth + 1, PCCPlayer[iCase] / mWidth};
         Coord CoordNewWall2 = {CoordNewWall1.X, CoordNewWall1.Y - 1};

         bConstructible = BuildVerticalWall (aPlayersDatas, aIdPlayer, aWallsBuilt, CoordNewWall1, CoordNewWall2, WallDatas, LengthPCC);
      }
      // Si la prochaine case est celle du haut
      else if (PCCPlayer[iCaseNext] == (PCCPlayer[iCase] - mWidth))
      {
         Coord CoordNewWall1 = {PCCPlayer[iCase] % mWidth, PCCPlayer[iCase] / mWidth};
         Coord CoordNewWall2 = {CoordNewWall1.X - 1, CoordNewWall1.Y};

         bConstructible = BuildHorizontalWall (aPlayersDatas, aIdPlayer, aWallsBuilt, CoordNewWall1, CoordNewWall2, WallDatas, LengthPCC);
      }
      // Si la prochaine case est celle du bas
      else if (PCCPlayer[iCaseNext] == (PCCPlayer[iCase] + mWidth))
      {
         Coord CoordNewWall1 = {PCCPlayer[iCase] % mWidth, PCCPlayer[iCase] / mWidth + 1};
         Coord CoordNewWall2 = {CoordNewWall1.X - 1, CoordNewWall1.Y};

         bConstructible = BuildHorizontalWall (aPlayersDatas, aIdPlayer, aWallsBuilt, CoordNewWall1, CoordNewWall2, WallDatas, LengthPCC);
      }

      if (bConstructible)
      {
//         cerr << "Mur " << WallDatas.ToString () << " L=" << LengthPCC << " (constructible)" << endl;

         if (LengthBestWall <= LengthPCC)
         {
//            cerr << "Best" << endl;
            LengthBestWall = LengthPCC;
            BestWallDatas  = WallDatas;

            WallBuilding = BestWallDatas.ToString ();
         }
      }
   }
   return WallBuilding;
}

bool CIA::BuildVerticalWall (CPlayerDatas::Vector& aPlayersDatas, const int& aIdPlayer, const vector<WallDatas>& aWallsBuilt, const Coord& aCoordWall1, const Coord& aCoordWall2, WallDatas& aWallDatas, unsigned int& aLengthPCC)
{
   bool bConstructible1 = IsConstructibleVertical (aWallsBuilt, aCoordWall1);
   bool bConstructible2 = IsConstructibleVertical (aWallsBuilt, aCoordWall2);

   WallDatas WallDatas1 = {aCoordWall1, "V"};
   WallDatas WallDatas2 = {aCoordWall2, "V"};
   unsigned int LengthPCC1 = 99;
   unsigned int LengthPCC2 = 99;

   if (bConstructible1 && bConstructible2)
   {
      // Calcul du nouveau PCC pour le joueur avec le Mur 1
      bConstructible1 = ChercheNewPCCPlayer (aPlayersDatas, aIdPlayer, WallDatas1, LengthPCC1);

      // Calcul du nouveau PCC pour le joueur avec le Mur 2
      bConstructible2 = ChercheNewPCCPlayer (aPlayersDatas, aIdPlayer, WallDatas2, LengthPCC2);

      // On Garde le meilleur
      SelectionneBestWall (bConstructible1, WallDatas1, LengthPCC1, bConstructible2, WallDatas2, LengthPCC2, aWallDatas, aLengthPCC);
   }
   else if (bConstructible1)
   {
      // Calcul du nouveau PCC pour le joueur avec le Mur 1
      bConstructible1 = ChercheNewPCCPlayer (aPlayersDatas, aIdPlayer, WallDatas1, LengthPCC1);

      if (bConstructible1)
      {
         aWallDatas = WallDatas1;
         aLengthPCC = LengthPCC1;
      }
   }
   else if (bConstructible2)
   {
      // Calcul du nouveau PCC pour le joueur avec le Mur 2
      bConstructible2 = ChercheNewPCCPlayer (aPlayersDatas, aIdPlayer, WallDatas2, LengthPCC2);

      if (bConstructible2)
      {
         aWallDatas = WallDatas2;
         aLengthPCC = LengthPCC2;
      }
   }
   return bConstructible1 || bConstructible2;
}

bool CIA::BuildHorizontalWall (CPlayerDatas::Vector& aPlayersDatas, const int& aIdPlayer, const vector<WallDatas>& aWallsBuilt, const Coord& aCoordWall1, const Coord& aCoordWall2, WallDatas& aWallDatas, unsigned int& aLengthPCC)
{
   bool bConstructible1 = IsConstructibleHorizontal (aWallsBuilt, aCoordWall1);
   bool bConstructible2 = IsConstructibleHorizontal (aWallsBuilt, aCoordWall2);

   WallDatas WallDatas1 = {aCoordWall1, "H"};
   WallDatas WallDatas2 = {aCoordWall2, "H"};
   unsigned int LengthPCC1 = 99;
   unsigned int LengthPCC2 = 99;

   if (bConstructible1 && bConstructible2)
   {
      // Calcul du nouveau PCC pour le joueur avec le Mur 1
      bConstructible1 = ChercheNewPCCPlayer (aPlayersDatas, aIdPlayer, WallDatas1, LengthPCC1);

      // Calcul du nouveau PCC pour le joueur avec le Mur 2
      bConstructible2 = ChercheNewPCCPlayer (aPlayersDatas, aIdPlayer, WallDatas2, LengthPCC2);

      // On Garde le meilleur
      SelectionneBestWall (bConstructible1, WallDatas1, LengthPCC1, bConstructible2, WallDatas2, LengthPCC2, aWallDatas, aLengthPCC);
   }
   else if (bConstructible1)
   {
      // Calcul du nouveau PCC pour le joueur avec le Mur 1
      bConstructible1 = ChercheNewPCCPlayer (aPlayersDatas, aIdPlayer, WallDatas1, LengthPCC1);

      if (bConstructible1)
      {
         aWallDatas = WallDatas1;
         aLengthPCC = LengthPCC1;
      }
   }
   else if (bConstructible2)
   {
      // Calcul du nouveau PCC pour le joueur avec le Mur 2
      bConstructible2 = ChercheNewPCCPlayer (aPlayersDatas, aIdPlayer, WallDatas2, LengthPCC2);

      if (bConstructible2)
      {
         aWallDatas = WallDatas2;
         aLengthPCC = LengthPCC2;
      }
   }
   return bConstructible1 || bConstructible2;
}

bool CIA::ChercheNewPCCPlayer (CPlayerDatas::Vector& aPlayersDatas, const int& aIdPlayer, const WallDatas& aWallDatas, unsigned int& aLengthPCC)
{
   bool bConstructible = true;
   AjoutMurMatriceGrapheLite (aWallDatas, false);
   CalculCheminMinimaux ();
   CPlayerDatas::Vector::iterator itPlayerDatas = aPlayersDatas.begin ();
   while (itPlayerDatas != aPlayersDatas.end () && bConstructible)
   {
      bConstructible = CalculPlusCourtCheminPlayer (*itPlayerDatas);
      ++itPlayerDatas;
   }
   if (bConstructible)
   {
      aLengthPCC = aPlayersDatas[aIdPlayer].GetPCC ().size ();
   }
   AjoutMurMatriceGrapheLite (aWallDatas, true);
   CalculCheminMinimaux ();
//   vector <int>::const_iterator IterPlusCourtChemin = aPlayersDatas[aIdPlayer].GetPCC ().begin ();
//   for (; IterPlusCourtChemin != aPlayersDatas[aIdPlayer].GetPCC ().end (); ++IterPlusCourtChemin)
//   {
//      cerr << (*IterPlusCourtChemin) << " ";
//   }
//   cerr << endl;
//   cerr << "Mur " << aWallDatas.ToString () << " L=" << aLengthPCC << endl;
   return bConstructible;
}

void CIA::SelectionneBestWall (const bool& abConstructible1, const WallDatas& aWallDatas1, const unsigned int& aLengthPCC1, const bool& abConstructible2, const WallDatas& aWallDatas2, const unsigned int& aLengthPCC2, WallDatas& aBestWallDatas, unsigned int& aLengthBestPCC)
{
   if (abConstructible1 && abConstructible2)
   {
      aBestWallDatas = aWallDatas1;
      aLengthBestPCC = aLengthPCC1;
      if (aLengthPCC1 < aLengthPCC2)
      {
         aBestWallDatas = aWallDatas2;
         aLengthBestPCC = aLengthPCC2;
      }
   }
   else if (abConstructible1)
   {
      aBestWallDatas = aWallDatas1;
      aLengthBestPCC = aLengthPCC1;
   }
   else if (abConstructible2)
   {
      aBestWallDatas = aWallDatas2;
      aLengthBestPCC = aLengthPCC2;
   }
}

bool CIA::IsConstructibleVertical (const vector<WallDatas>& aWallsBuilt, const Coord aNewWall) const
{
    bool bConstructible = true;
    
    if ((0 <= aNewWall.Y) && (aNewWall.Y <= (mHeight - 2)))
    {
        vector<WallDatas>::const_iterator itWallDatas = aWallsBuilt.begin ();
        while ((itWallDatas != aWallsBuilt.end ()) && bConstructible)
        {
            if (itWallDatas->Position.X == aNewWall.X)
            {
                if ((0 == itWallDatas->Orientation.compare(string("V"))) && ((itWallDatas->Position.Y == aNewWall.Y) || ((itWallDatas->Position.Y - 1) == aNewWall.Y) || ((itWallDatas->Position.Y + 1) == aNewWall.Y)))
                {
                    bConstructible = false;
                }
            }
            else if (((itWallDatas->Position.X + 1) == aNewWall.X) && ((itWallDatas->Position.Y - 1) == aNewWall.Y) && (0 == itWallDatas->Orientation.compare(string("H"))))
            {
                bConstructible = false;
            }
            
            ++itWallDatas;
        }
    }
    else
    {
        bConstructible = false;
    }
    
    return bConstructible;
}

bool CIA::IsConstructibleHorizontal (const vector<WallDatas>& aWallsBuilt, const Coord aNewWall) const
{
    bool bConstructible = true;
    
    if ((0 <= aNewWall.X) && (aNewWall.X <= (mWidth - 2)))
    {
        vector<WallDatas>::const_iterator itWallDatas = aWallsBuilt.begin ();
        while ((itWallDatas != aWallsBuilt.end ()) && bConstructible)
        {
            if (itWallDatas->Position.Y == aNewWall.Y)
            {
                if ((0 == itWallDatas->Orientation.compare(string("H"))) && ((itWallDatas->Position.X == aNewWall.X) || ((itWallDatas->Position.X - 1) == aNewWall.X) || ((itWallDatas->Position.X + 1) == aNewWall.X)))
                {
                    bConstructible = false;
                }
            }
            else if (((itWallDatas->Position.Y + 1) == aNewWall.Y) && ((itWallDatas->Position.X - 1) == aNewWall.X) && (0 == itWallDatas->Orientation.compare(string("V"))))
            {
                bConstructible = false;
            }
            
            ++itWallDatas;
        }
    }
    else
    {
        bConstructible = false;
    }
    
    return bConstructible;
}


bool CIA::IsCheminPossiblePlayer (const CPlayerDatas& aPlayersDatas) const
{
    bool bCheminExiste = false;
    
    if ((aPlayersDatas.GetPosition ().X == -1) || (aPlayersDatas.GetPosition ().Y == -1) || (aPlayersDatas.GetDirection () == -1))
    {
        bCheminExiste = true;
    }
    else
    {
        vector<int> CasesArrivees;
        switch (aPlayersDatas.GetDirection ())
        {
            case eRight:
                for (int i=0; i<mHeight; ++i)
                    CasesArrivees.push_back ((mWidth - 1) + i * mWidth);
                break;
            case eLeft:
                for (int i=0; i<mHeight; ++i)
                    CasesArrivees.push_back (i * mWidth);
                break;
            case eDown:
                for (int i=0; i<mWidth; ++i)
                    CasesArrivees.push_back (i + (mHeight - 1) * mWidth);
                break;
            case eUp:
            default:
                break;
        }

        unsigned int i = 0;
        while ((false == bCheminExiste) && (i < CasesArrivees.size()))
        {
            bCheminExiste = IsCheminPossible (aPlayersDatas.GetPosition ().X + aPlayersDatas.GetPosition ().Y * mWidth, CasesArrivees[i]);
            ++i;
        }
    }
    return bCheminExiste;
}

bool CIA::IsCheminPossible (const int aNumCaseDepart, const int aNumCaseArrivee) const
{
    // Récupération du plus court chemin
    int NumCaseCourante = aNumCaseDepart;
    int NbIterMax = mHeight * mWidth;
    int NbIter = 0;
    while ((NbIter != NbIterMax) && (NumCaseCourante != aNumCaseArrivee) && (NumCaseCourante < POIDS_MUR))
    {
        NumCaseCourante = mCheminsMinimaux[aNumCaseArrivee][NumCaseCourante];
        ++NbIter;
    }
    return ((NbIter != NbIterMax) && (NumCaseCourante < POIDS_MUR));
}

void CIA::AjoutMurMatriceGraphe (const WallDatas& aWallDatas)
{
    const bool bVertical = (0 == aWallDatas.Orientation.compare(string("V")));
    
    if (bVertical && (aWallDatas.Position.Y != mHeight))
    {
        if ((aWallDatas.Position.X != 0) && (aWallDatas.Position.X != mWidth))
        {
            int NumCase1 = aWallDatas.Position.X - 1 + aWallDatas.Position.Y * mWidth;
            int NumCase2 = aWallDatas.Position.X - 1 + (aWallDatas.Position.Y + 1) * mWidth;
            
             mMatriceGraph[NumCase1][NumCase1 + 1] *= POIDS_MUR;
             mMatriceGraph[NumCase1 + 1][NumCase1] *= POIDS_MUR;
             mMatriceGraph[NumCase2][NumCase2 + 1] *= POIDS_MUR;
             mMatriceGraph[NumCase2 + 1][NumCase2] *= POIDS_MUR;
        }
    }
    else if ((false == bVertical) && (aWallDatas.Position.X != mWidth))
    {
        if ((aWallDatas.Position.Y != 0) &&  (aWallDatas.Position.Y != mHeight))
        {
            int NumCase1 = aWallDatas.Position.X + (aWallDatas.Position.Y - 1) * mWidth;
            int NumCase2 = aWallDatas.Position.X + 1 + (aWallDatas.Position.Y - 1) * mWidth;
            
             mMatriceGraph[NumCase1][NumCase1 + mWidth] *= POIDS_MUR;
             mMatriceGraph[NumCase1 + mWidth][NumCase1] *= POIDS_MUR;
             mMatriceGraph[NumCase2][NumCase2 + mWidth] *= POIDS_MUR;
             mMatriceGraph[NumCase2 + mWidth][NumCase2] *= POIDS_MUR;
        }
    }
}

void CIA::AjoutMurMatriceGrapheLite (const WallDatas& aWallDatas, const bool abDestroy)
{
   const bool bVertical = (0 == aWallDatas.Orientation.compare(string("V")));

   if (bVertical && (aWallDatas.Position.Y != mHeight))
   {
       if ((aWallDatas.Position.X != 0) && (aWallDatas.Position.X != mWidth))
       {
           int NumCase1 = aWallDatas.Position.X - 1 + aWallDatas.Position.Y * mWidth;
           int NumCase2 = aWallDatas.Position.X - 1 + (aWallDatas.Position.Y + 1) * mWidth;

           if (abDestroy)
           {
               mMatriceGraph[NumCase1][NumCase1 + 1] /= POIDS_MUR;
               mMatriceGraph[NumCase1 + 1][NumCase1] /= POIDS_MUR;
               mMatriceGraph[NumCase2][NumCase2 + 1] /= POIDS_MUR;
               mMatriceGraph[NumCase2 + 1][NumCase2] /= POIDS_MUR;
           }
           else
           {
               mMatriceGraph[NumCase1][NumCase1 + 1] *= POIDS_MUR;
               mMatriceGraph[NumCase1 + 1][NumCase1] *= POIDS_MUR;
               mMatriceGraph[NumCase2][NumCase2 + 1] *= POIDS_MUR;
               mMatriceGraph[NumCase2 + 1][NumCase2] *= POIDS_MUR;
           }
       }
   }
   else if ((false == bVertical) && (aWallDatas.Position.X != mWidth))
   {
       if ((aWallDatas.Position.Y != 0) &&  (aWallDatas.Position.Y != mHeight))
       {
           int NumCase1 = aWallDatas.Position.X + (aWallDatas.Position.Y - 1) * mWidth;
           int NumCase2 = aWallDatas.Position.X + 1 + (aWallDatas.Position.Y - 1) * mWidth;

           if (abDestroy)
           {
               mMatriceGraph[NumCase1][NumCase1 + mWidth] /= POIDS_MUR;
               mMatriceGraph[NumCase1 + mWidth][NumCase1] /= POIDS_MUR;
               mMatriceGraph[NumCase2][NumCase2 + mWidth] /= POIDS_MUR;
               mMatriceGraph[NumCase2 + mWidth][NumCase2] /= POIDS_MUR;
           }
           else
           {
               mMatriceGraph[NumCase1][NumCase1 + mWidth] *= POIDS_MUR;
               mMatriceGraph[NumCase1 + mWidth][NumCase1] *= POIDS_MUR;
               mMatriceGraph[NumCase2][NumCase2 + mWidth] *= POIDS_MUR;
               mMatriceGraph[NumCase2 + mWidth][NumCase2] *= POIDS_MUR;
           }
       }
   }
}

/**
 * Auto-generated code below aims at helping you parse
 * the standard input according to the problem statement.
 **/
int main()
{
    int w; // width of the board
    int h; // height of the board
    int playerCount; // number of players (2 or 3)
    int myId; // id of my player (0 = 1st player, 1 = 2nd player, ...)
    cin >> w >> h >> playerCount >> myId; cin.ignore();
    
    CPlayerDatas::Vector   PlayersDatas;
    vector<WallDatas>      WallsDatas;
    bool                   bBuildingOn = false;
    
    CIA IA (w, h);
    
    // game loop
    while (1)
    {
        IA.ConstruireMatriceGraphe ();

        unsigned int Marge = playerCount - 1;
        Marge = 0;
        PlayersDatas.clear ();
        WallsDatas.clear ();
        for (int i = 0; i < playerCount; i++)
        {
            int x; // x-coordinate of the player
            int y; // y-coordinate of the player
            int wallsLeft; // number of walls available for the player
            cin >> x >> y >> wallsLeft; cin.ignore();
            cerr << x << " " << y << " " << wallsLeft << endl;
            PlayersDatas.push_back (CPlayerDatas(static_cast<EDirection>(i), Coord{x, y}, wallsLeft));
        }
        int wallCount; // number of walls on the board
        cin >> wallCount; cin.ignore();
        for (int i = 0; i < wallCount; i++)
        {
            int wallX; // x-coordinate of the wall
            int wallY; // y-coordinate of the wall
            string wallOrientation; // wall orientation ('H' or 'V')
            cin >> wallX >> wallY >> wallOrientation; cin.ignore();
            
            WallsDatas.push_back ({wallX, wallY, wallOrientation});
            IA.AjoutMurMatriceGraphe ({wallX, wallY, wallOrientation});
        }
        
        IA.CalculCheminMinimaux ();
        
        // Write an action using cout. DON'T FORGET THE "<< endl"
        // To debug: cerr << "Debug messages..." << endl;

        int    NbPlayerActif = 0;
        bool   bAvance = true;
        string Action;

        int IdPremierPlayer = -1;
        int IdDernierPlayer = -1;
        unsigned int DistancePremierPlayer = 999;
        unsigned int DistanceDernierPlayer = 0;

        for (int i = 0; i < playerCount; i++)
        {
            IA.CalculPlusCourtCheminPlayer (PlayersDatas[i]);

            if (PlayersDatas[i].Play ())
            {
               ++NbPlayerActif;
               if ((PlayersDatas[i].GetPCC ().size() < DistancePremierPlayer))
               {
                  IdPremierPlayer = i;
                  DistancePremierPlayer = PlayersDatas[i].GetPCC ().size();
               }
               else if (DistanceDernierPlayer < PlayersDatas[i].GetPCC ().size())
               {
                  IdDernierPlayer = i;
                  DistanceDernierPlayer = PlayersDatas[i].GetPCC ().size();
               }
            }
        }
        cerr << "F=" << IdPremierPlayer << " ,L=" << IdDernierPlayer << endl;

        // 2 joueurs
        if (NbPlayerActif == 2)
        {
           // Je ne peux plus construire de mur OU Je suis le premier OU Je suis à la même distance que le premier et je suis le premier en ordre de jeu
           bAvance =  (PlayersDatas[myId].GetNbWallsLeft () == 0);
           bAvance |= (IdPremierPlayer == myId);
           bAvance |= ((PlayersDatas[myId].GetPCC ().size () == (DistancePremierPlayer - Marge)) && (myId < IdPremierPlayer));
           bAvance |= ((IdPremierPlayer != myId) && ((DistancePremierPlayer > 2) && !bBuildingOn));
        }
        // 3 joueurs
        else if (NbPlayerActif == 3)
        {
           // Je ne peux plus construire de mur OU Je suis ne suis pas le dernier OU Je suis à la même distance que le premier et je suis le premier en ordre de jeu
           bAvance =  (PlayersDatas[myId].GetNbWallsLeft () == 0);
           bAvance |= (IdDernierPlayer != myId);
           bAvance |= ((PlayersDatas[myId].GetPCC ().size () == (DistanceDernierPlayer - Marge)) && (myId < IdDernierPlayer));
           bAvance |= ((IdPremierPlayer != myId) && ((DistancePremierPlayer > 2) && !bBuildingOn));
        }

        if (bAvance)
        {
            cerr << "G" << endl;
            Action = IA.GetNextDirection (PlayersDatas[myId].GetPCC ());
        }
        else
        {
            cerr << "B" << endl;
            bBuildingOn = true;
            Action = IA.BuildWallInFrontOfPlayer (PlayersDatas, IdPremierPlayer, WallsDatas);
            
            if (Action.empty ())
            {
                Action = IA.GetNextDirection (PlayersDatas[myId].GetPCC ());
            }
        }

        cout << Action << endl; // action: LEFT, RIGHT, UP, DOWN or "putX putY putOrientation" to place a wall
    }
}
