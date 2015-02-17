#include <iostream>
#include <string>
#include <vector>
#include <list>
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
#define COEF_POIDS_CHEMIN       1

static bool _bFirstWall = true;
static int  _myId = -1; // id of my player (0 = 1st player, 1 = 2nd player, ...)

class CPlayerDatas
{
public:
   typedef vector<CPlayerDatas> Vector;
   typedef list<CPlayerDatas> List;
public:
   CPlayerDatas (const int& aId,const Coord& aPosition, const int& aWallsLeft):
      mId         (aId),
      mDirection  (static_cast<EDirection>(aId)),
      mPosition   (aPosition),
      mWallsLeft  (aWallsLeft),
      mbPlay      (true)
   {};
   ~CPlayerDatas (void) {};

   inline bool operator== (const CPlayerDatas& aPlayer) {
         return (mId == aPlayer.mId);
   };

   inline bool operator!= (const CPlayerDatas& aPlayer) {
      return (mId != aPlayer.mId);
   };

   inline const int GetId (void) const {
      return mId;
   };

   inline const EDirection GetDirection (void) const {
      return mDirection;
   };

   inline const Coord GetPosition (void) const {
      return mPosition;
   };

   inline const int GetNbWallsLeft (void) const {
      return mWallsLeft;
   };

   inline const vector<int>& GetPCC (void) const {
      return mPCC;
   };

   inline void SetPCC (const vector<int>& aPCC) {
      mPCC = aPCC;
   };

   inline bool Play (void) const {
      return mbPlay;
   };

   inline void SetOut (void) {
      mbPlay = false;
   };

private:
   const int         mId;
   const EDirection  mDirection;
   const Coord       mPosition;
   const int         mWallsLeft;

   bool              mbPlay;
   vector<int>       mPCC;
};

bool ComparePlayer (const CPlayerDatas& aPlayer1, const CPlayerDatas& aPlayer2)
{
   bool bReturn = false;
   if (aPlayer1.GetPCC ().size () < aPlayer2.GetPCC ().size ()) {
      bReturn = true;
   }
   else if (aPlayer1.GetPCC ().size () == aPlayer2.GetPCC ().size ()) {
      if (_myId == aPlayer1.GetId ()) {
         bReturn = true;
      }
      else if (_myId == aPlayer2.GetId ()) {
         bReturn = false;
      }
      else {
         if ((_myId < aPlayer1.GetId ()) || (aPlayer1.GetId () == 0)) {
            bReturn = true;
         }
      }
   }
   return bReturn;
}


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
    string  BuildWallInFrontOfPlayer    (CPlayerDatas::List& aPlayersDatas, CPlayerDatas& aPlayer, const vector<WallDatas>& aWallsBuilt);

private:
    bool CalculPlusCourtChemin  (const int aNumCaseDepart, const int aNumCaseArrivee, vector<int>& aOutPlusCourtChemin);
    
    bool BuildVerticalWall    (CPlayerDatas::List& aPlayersDatas, CPlayerDatas& aPlayer, const vector<WallDatas>& aWallsBuilt, const Coord& aCoordWall1, const Coord& aCoordWall2, WallDatas& aWallDatas, unsigned int& aLengthPCC);
    bool BuildHorizontalWall  (CPlayerDatas::List& aPlayersDatas, CPlayerDatas& aPlayer, const vector<WallDatas>& aWallsBuilt, const Coord& aCoordWall1, const Coord& aCoordWall2, WallDatas& aWallDatas, unsigned int& aLengthPCC);

    bool ChercheNewPCCPlayer (CPlayerDatas::List& aPlayersDatas, CPlayerDatas& aPlayer, const WallDatas& aWallDatas, unsigned int& aLengthPCC);
    void SelectionneBestWall (const CPlayerDatas& aPlayerDatas, const bool& abConstructible1, const WallDatas& aWallDatas1, const unsigned int& aLengthPCC1, const bool& abConstructible2, const WallDatas& aWallDatas2, const unsigned int& aLengthPCC2, WallDatas& aBestWallDatas, unsigned int& aLengthBestPCC);

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
   return (DistanceMin != 99);
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

string CIA::BuildWallInFrontOfPlayer (CPlayerDatas::List& aPlayersDatas, CPlayerDatas& aPlayer, const vector<WallDatas>& aWallsBuilt)
{
   const vector<int> PCCPlayer = aPlayer.GetPCC ();

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

         bConstructible = BuildVerticalWall (aPlayersDatas, aPlayer, aWallsBuilt, CoordNewWall1, CoordNewWall2, WallDatas, LengthPCC);
      }
      // Si la prochaine case est celle de droite
      else if (PCCPlayer[iCaseNext] == (PCCPlayer[iCase] + 1))
      {
         Coord CoordNewWall1 = {PCCPlayer[iCase] % mWidth + 1, PCCPlayer[iCase] / mWidth};
         Coord CoordNewWall2 = {CoordNewWall1.X, CoordNewWall1.Y - 1};

         bConstructible = BuildVerticalWall (aPlayersDatas, aPlayer, aWallsBuilt, CoordNewWall1, CoordNewWall2, WallDatas, LengthPCC);
      }
      // Si la prochaine case est celle du haut
      else if (PCCPlayer[iCaseNext] == (PCCPlayer[iCase] - mWidth))
      {
         Coord CoordNewWall1 = {PCCPlayer[iCase] % mWidth, PCCPlayer[iCase] / mWidth};
         Coord CoordNewWall2 = {CoordNewWall1.X - 1, CoordNewWall1.Y};

         bConstructible = BuildHorizontalWall (aPlayersDatas, aPlayer, aWallsBuilt, CoordNewWall1, CoordNewWall2, WallDatas, LengthPCC);
      }
      // Si la prochaine case est celle du bas
      else if (PCCPlayer[iCaseNext] == (PCCPlayer[iCase] + mWidth))
      {
         Coord CoordNewWall1 = {PCCPlayer[iCase] % mWidth, PCCPlayer[iCase] / mWidth + 1};
         Coord CoordNewWall2 = {CoordNewWall1.X - 1, CoordNewWall1.Y};

         bConstructible = BuildHorizontalWall (aPlayersDatas, aPlayer, aWallsBuilt, CoordNewWall1, CoordNewWall2, WallDatas, LengthPCC);
      }

      if (bConstructible)
      {
//         cerr << "Mur " << WallDatas.ToString () << " L=" << LengthPCC << " (constructible)" << endl;

         if (LengthBestWall < LengthPCC)
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

bool CIA::BuildVerticalWall (CPlayerDatas::List& aPlayersDatas, CPlayerDatas& aPlayer, const vector<WallDatas>& aWallsBuilt, const Coord& aCoordWall1, const Coord& aCoordWall2, WallDatas& aWallDatas, unsigned int& aLengthPCC)
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
      bConstructible1 = ChercheNewPCCPlayer (aPlayersDatas, aPlayer, WallDatas1, LengthPCC1);

      // Calcul du nouveau PCC pour le joueur avec le Mur 2
      bConstructible2 = ChercheNewPCCPlayer (aPlayersDatas, aPlayer, WallDatas2, LengthPCC2);

      // On sélectionne le meilleur
      SelectionneBestWall (aPlayer, bConstructible1, WallDatas1, LengthPCC1, bConstructible2, WallDatas2, LengthPCC2, aWallDatas, aLengthPCC);
   }
   else if (bConstructible1)
   {
      // Calcul du nouveau PCC pour le joueur avec le Mur 1
      bConstructible1 = ChercheNewPCCPlayer (aPlayersDatas, aPlayer, WallDatas1, LengthPCC1);

      if (bConstructible1)
      {
         aWallDatas = WallDatas1;
         aLengthPCC = LengthPCC1;
      }
   }
   else if (bConstructible2)
   {
      // Calcul du nouveau PCC pour le joueur avec le Mur 2
      bConstructible2 = ChercheNewPCCPlayer (aPlayersDatas, aPlayer, WallDatas2, LengthPCC2);

      if (bConstructible2)
      {
         aWallDatas = WallDatas2;
         aLengthPCC = LengthPCC2;
      }
   }
   return bConstructible1 || bConstructible2;
}

bool CIA::BuildHorizontalWall (CPlayerDatas::List& aPlayersDatas, CPlayerDatas& aPlayer, const vector<WallDatas>& aWallsBuilt, const Coord& aCoordWall1, const Coord& aCoordWall2, WallDatas& aWallDatas, unsigned int& aLengthPCC)
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
      bConstructible1 = ChercheNewPCCPlayer (aPlayersDatas, aPlayer, WallDatas1, LengthPCC1);

      // Calcul du nouveau PCC pour le joueur avec le Mur 2
      bConstructible2 = ChercheNewPCCPlayer (aPlayersDatas, aPlayer, WallDatas2, LengthPCC2);

      // On Garde le meilleur
      SelectionneBestWall (aPlayer, bConstructible1, WallDatas1, LengthPCC1, bConstructible2, WallDatas2, LengthPCC2, aWallDatas, aLengthPCC);
   }
   else if (bConstructible1)
   {
      // Calcul du nouveau PCC pour le joueur avec le Mur 1
      bConstructible1 = ChercheNewPCCPlayer (aPlayersDatas, aPlayer, WallDatas1, LengthPCC1);

      if (bConstructible1)
      {
         aWallDatas = WallDatas1;
         aLengthPCC = LengthPCC1;
      }
   }
   else if (bConstructible2)
   {
      // Calcul du nouveau PCC pour le joueur avec le Mur 2
      bConstructible2 = ChercheNewPCCPlayer (aPlayersDatas, aPlayer, WallDatas2, LengthPCC2);

      if (bConstructible2)
      {
         aWallDatas = WallDatas2;
         aLengthPCC = LengthPCC2;
      }
   }
   return bConstructible1 || bConstructible2;
}

bool CIA::ChercheNewPCCPlayer (CPlayerDatas::List& aPlayersDatas, CPlayerDatas& aPlayer, const WallDatas& aWallDatas, unsigned int& aLengthPCC)
{
   bool bConstructible = true;
   AjoutMurMatriceGrapheLite (aWallDatas, false);
   CalculCheminMinimaux ();
   CPlayerDatas::List::iterator itPlayerDatas = aPlayersDatas.begin ();
   while (itPlayerDatas != aPlayersDatas.end () && bConstructible)
   {
      bConstructible = CalculPlusCourtCheminPlayer (*itPlayerDatas);
      ++itPlayerDatas;
   }
   if (bConstructible)
   {
      aLengthPCC = aPlayer.GetPCC ().size ();
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

void CIA::SelectionneBestWall (const CPlayerDatas& aPlayerDatas, const bool& abConstructible1, const WallDatas& aWallDatas1, const unsigned int& aLengthPCC1, const bool& abConstructible2, const WallDatas& aWallDatas2, const unsigned int& aLengthPCC2, WallDatas& aBestWallDatas, unsigned int& aLengthBestPCC)
{
   if (abConstructible1 && abConstructible2)
   {
      if (_bFirstWall)
      {
         if ((aPlayerDatas.GetDirection () == eRight) || (aPlayerDatas.GetDirection () == eLeft))
         {
            if (aPlayerDatas.GetPosition ().Y < ((mHeight - 1) / 2))
            {
               if (0 == (aWallDatas1.Position.Y % 2))
               {
                  aBestWallDatas = aWallDatas1;
                  aLengthBestPCC = aLengthPCC1;
               }
               else
               {
                  aBestWallDatas = aWallDatas2;
                  aLengthBestPCC = aLengthPCC2;
               }
            }
            else
            {
               if (0 == (aWallDatas1.Position.Y % 2))
               {
                  aBestWallDatas = aWallDatas2;
                  aLengthBestPCC = aLengthPCC2;
               }
               else
               {
                  aBestWallDatas = aWallDatas1;
                  aLengthBestPCC = aLengthPCC1;
               }
            }
         }
         else
         {
            if (aPlayerDatas.GetPosition ().X < ((mWidth - 1) / 2))
            {
               if (0 == (aWallDatas1.Position.X % 2))
               {
                  aBestWallDatas = aWallDatas1;
                  aLengthBestPCC = aLengthPCC1;
               }
               else
               {
                  aBestWallDatas = aWallDatas2;
                  aLengthBestPCC = aLengthPCC2;
               }
            }
            else
            {
               if (0 == (aWallDatas1.Position.X % 2))
               {
                  aBestWallDatas = aWallDatas2;
                  aLengthBestPCC = aLengthPCC2;
               }
               else
               {
                  aBestWallDatas = aWallDatas1;
                  aLengthBestPCC = aLengthPCC1;
               }
            }
         }
      }
      else
      {
         aBestWallDatas = aWallDatas1;
         aLengthBestPCC = aLengthPCC1;
         if (aLengthPCC1 < aLengthPCC2)
         {
            aBestWallDatas = aWallDatas2;
            aLengthBestPCC = aLengthPCC2;
         }
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
   cin >> w >> h >> playerCount >> _myId; cin.ignore();

   CPlayerDatas::List     PlayersDatasOrdonnes;
   vector<WallDatas>      WallsDatas;
   bool                   bBuildingOn = false;

   CIA IA (w, h);

   // game loop
   while (1) {
      IA.ConstruireMatriceGraphe ();

      unsigned int Marge = playerCount - 1;
      Marge = 0;
      PlayersDatasOrdonnes.clear();
      WallsDatas.clear ();
      for (int i = 0; i < playerCount; i++) {
         int x; // x-coordinate of the player
         int y; // y-coordinate of the player
         int wallsLeft; // number of walls available for the player
         cin >> x >> y >> wallsLeft; cin.ignore();
         cerr << x << " " << y << " " << wallsLeft << endl;

         if ((x != -1) && (y != -1) && (wallsLeft != -1)) {
            PlayersDatasOrdonnes.push_back (CPlayerDatas(static_cast<EDirection>(i), Coord{x, y}, wallsLeft));
         }
      }
      int wallCount; // number of walls on the board
      cin >> wallCount; cin.ignore();
      for (int i = 0; i < wallCount; i++) {
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

      bool   bAvance = true;
      string Action;

      CPlayerDatas::List::iterator ItPlayerOrdonnes;
      for (ItPlayerOrdonnes = PlayersDatasOrdonnes.begin ();
           ItPlayerOrdonnes != PlayersDatasOrdonnes.end ();
           ++ItPlayerOrdonnes) {
         IA.CalculPlusCourtCheminPlayer (*ItPlayerOrdonnes);
      }
      PlayersDatasOrdonnes.sort (ComparePlayer);

      /*
      for (ItPlayerOrdonnes = PlayersDatasOrdonnes.begin ();
           ItPlayerOrdonnes != PlayersDatasOrdonnes.end ();
           ++ItPlayerOrdonnes)
      {
         cerr << "[" << ItPlayerOrdonnes->GetId () << "] (" << ItPlayerOrdonnes->GetPCC ().size() << ") ";
      }
      cerr << endl;*/

      CPlayerDatas::List::iterator Me;
      for (ItPlayerOrdonnes = PlayersDatasOrdonnes.begin ();
           ItPlayerOrdonnes != PlayersDatasOrdonnes.end ();
           ++ItPlayerOrdonnes) {
         if (ItPlayerOrdonnes->GetId() == _myId) {
            Me = ItPlayerOrdonnes;
         }
      }

      // 2 joueurs
      if (PlayersDatasOrdonnes.size () == 2) {
         // Je ne peux plus construire de mur OU Je suis le premier OU Je suis à la même distance que le premier et je suis le premier en ordre de jeu
         bAvance =  ((*Me).GetNbWallsLeft () == 0);
         bAvance |= ((*Me) == PlayersDatasOrdonnes.front ());
         bAvance |= ((*Me).GetPCC ().size () == (PlayersDatasOrdonnes.front ().GetPCC ().size () - Marge));
         bAvance |= (((*Me) != PlayersDatasOrdonnes.front ()) && ((PlayersDatasOrdonnes.front ().GetPCC ().size () > 2) && !bBuildingOn));
      }
      // 3 joueurs
      else if (PlayersDatasOrdonnes.size () == 3) {
         // Je ne peux plus construire de mur OU Je suis ne suis pas le dernier OU Je suis à la même distance que le premier et je suis le premier en ordre de jeu
         bAvance =  ((*Me).GetNbWallsLeft () == 0);
         bAvance |= ((*Me) != PlayersDatasOrdonnes.back ());
         bAvance |= (((*Me) != PlayersDatasOrdonnes.back ()) && ((*Me).GetPCC ().size () == (PlayersDatasOrdonnes.back ().GetPCC ().size () - Marge)));
         bAvance |= (((*Me) != PlayersDatasOrdonnes.front ()) && ((PlayersDatasOrdonnes.front ().GetPCC ().size () > 2) && !bBuildingOn));
         bAvance |= (((*Me) != PlayersDatasOrdonnes.back ()) && ((*Me) != PlayersDatasOrdonnes.front ()) && (PlayersDatasOrdonnes.back ().GetNbWallsLeft () != 0));
      }

      if (bAvance) {
         cerr << "G" << endl;
         Action = IA.GetNextDirection ((*Me).GetPCC ());
      }
      else {
         cerr << "B" << endl;
         bBuildingOn = true;

//         Action = IA.BuildWallInFrontOfPlayer (PlayersDatasOrdonnes, PlayersDatasOrdonnes.front (), WallsDatas);
         CPlayerDatas::List::iterator ItPlayer = PlayersDatasOrdonnes.begin ();
         while ((ItPlayer != Me) && (Action.empty ())) {
            Action = IA.BuildWallInFrontOfPlayer (PlayersDatasOrdonnes, *ItPlayer, WallsDatas);
            ++ItPlayer;
         }

         if (Action.empty ()) {
            Action = IA.GetNextDirection ((*Me).GetPCC ());
         }
         else {
            _bFirstWall = false;
         }
      }

      cout << Action << endl; // action: LEFT, RIGHT, UP, DOWN or "putX putY putOrientation" to place a wall
   }
}
