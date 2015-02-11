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

struct PlayerDatas
{
    EDirection  Direction;
    int         PositionX;
    int         PositionY;
    int         WallsLeft;
};

struct WallDatas
{
    int     PositionX;
    int     PositionY;
    string  Orientation;
};

#define POIDS_MUR               9999
#define COEF_POIDS_CHEMIN       2

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
    void    AjoutMurMatriceGrapheLite     (const WallDatas& aWallDatas, const bool abDestroy = false);
    bool    CalculPlusCourtCheminPlayer   (const PlayerDatas& aPlayersDatas, vector<int>& aOutPCC);
    void    CalculCheminMinimaux          (void);
    
    string  GetNextDirection            (const vector<int>& aPlusCourtChemin);
    string  BuildWall                   (const vector<PlayerDatas>& aPlayersDatas, const vector<int>& aPlusCourtChemin, const vector<WallDatas>& aWallsBuilt);

private:
    bool CalculPlusCourtChemin  (const int aNumCaseDepart, const int aNumCaseArrivee, vector<int>& aOutPlusCourtChemin);
    
    bool IsConstructibleVertical    (const vector<WallDatas>& aWallsBuilt, const int aNewWallX, const int aNewWallY) const;
    bool IsConstructibleHorizontal  (const vector<WallDatas>& aWallsBuilt, const int aNewWallX, const int aNewWallY) const;
    
    bool IsCheminPossiblePlayer (const PlayerDatas& aPlayersDatas) const;
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

bool CIA::CalculPlusCourtCheminPlayer (const PlayerDatas& aPlayersDatas, vector<int>& aOutPCC)
{
    bool bReturn = false;
    if ((aPlayersDatas.PositionX == -1) || (aPlayersDatas.PositionY == -1) || (aPlayersDatas.Direction == -1))
    {
        for (int i = 0; i < (mWidth * mHeight); ++i)
        {
            aOutPCC.push_back (i);
        }
        bReturn = true;
    }
    else
    {
        vector<int> CasesArrivees;
        switch (aPlayersDatas.Direction)
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
            bReturn = CalculPlusCourtChemin (aPlayersDatas.PositionX + aPlayersDatas.PositionY * mWidth, CasesArrivees[i], PCC);
            if (bReturn && (1 < PCC.size()) && (PCC.size() < DistanceMin))
            {
                DistanceMin = PCC.size();
                aOutPCC = PCC;
            }
        }
    }
    return bReturn;
}

bool CIA::CalculPlusCourtChemin (const int aNumCaseDepart, const int aNumCaseArrivee, vector<int>& aOutPlusCourtChemin)
{
    cerr << "[" << aNumCaseDepart << ", " << aNumCaseArrivee << "]" << endl;

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

    if (aOutPlusCourtChemin.size () < (mWidth * mHeight))
    {
        vector <int>::const_iterator IterPlusCourtChemin = aOutPlusCourtChemin.begin ();
        for (; IterPlusCourtChemin != aOutPlusCourtChemin.end (); ++IterPlusCourtChemin)
        {
            cerr << (*IterPlusCourtChemin) << " ";
        }
        cerr << endl;
    }
    cerr << "=>" << aOutPlusCourtChemin.size () << endl;
    
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

string CIA::BuildWall (const vector<PlayerDatas>& aPlayersDatas, const vector<int>& aPlusCourtChemin, const vector<WallDatas>& aWallsBuilt)
{
    string WallBuilding;
    
    unsigned int iCase = 0;
    unsigned int iCaseNext = 1;
    
    while (WallBuilding.empty () && (iCaseNext < aPlusCourtChemin.size ()))
    {
        bool bConstructible = false;
        WallDatas WallDatas;
        // Si la prochaine case est celle de gauche
        if (aPlusCourtChemin[iCaseNext] == (aPlusCourtChemin[iCase] - 1))
        {
            int NewWallX = aPlusCourtChemin[iCase] % mWidth;
            int NewWallY = aPlusCourtChemin[iCase] / mWidth;
            
            bConstructible = IsConstructibleVertical (aWallsBuilt, NewWallX, NewWallY);
            if (bConstructible)
            {
                WallBuilding = to_string(NewWallX) + " " + to_string(NewWallY) + " V";
                WallDatas = {NewWallX, NewWallY, "V"};
            }
            else
            {
                --NewWallY;
                bConstructible = IsConstructibleVertical (aWallsBuilt, NewWallX, NewWallY);
                if (bConstructible)
                {
                    WallBuilding = to_string(NewWallX) + " " + to_string(NewWallY) + " V";
                    WallDatas = {NewWallX, NewWallY, "V"};
                }
            }
        }
        // Si la prochaine case est celle de droite
        else if (aPlusCourtChemin[iCaseNext] == (aPlusCourtChemin[iCase] + 1))
        {
            int NewWallX = aPlusCourtChemin[iCase] % mWidth + 1;
            int NewWallY = aPlusCourtChemin[iCase] / mWidth;
            
            bConstructible = IsConstructibleVertical (aWallsBuilt, NewWallX, NewWallY);
            if (bConstructible)
            {
                WallBuilding = to_string(NewWallX) + " " + to_string(NewWallY) + " V";
                WallDatas = {NewWallX, NewWallY, "V"};
            }
            else
            {
                --NewWallY;
                bConstructible = IsConstructibleVertical (aWallsBuilt, NewWallX, NewWallY);
                if (bConstructible)
                {
                    WallBuilding = to_string(NewWallX) + " " + to_string(NewWallY) + " V";
                    WallDatas = {NewWallX, NewWallY, "V"};
                }
            }
        }
        // Si la prochaine case est celle du haut
        else if (aPlusCourtChemin[iCaseNext] == (aPlusCourtChemin[iCase] - mWidth))
        {
            int NewWallX = aPlusCourtChemin[iCase] % mWidth;
            int NewWallY = aPlusCourtChemin[iCase] / mWidth;
            
            bConstructible = IsConstructibleHorizontal (aWallsBuilt, NewWallX, NewWallY);
            if (bConstructible)
            {
                WallBuilding = to_string(NewWallX) + " " + to_string(NewWallY) + " H";
                WallDatas = {NewWallX, NewWallY, "H"};
            }
            else
            {
                --NewWallX;
                bConstructible = IsConstructibleHorizontal (aWallsBuilt, NewWallX, NewWallY);
                if (bConstructible)
                {
                    WallBuilding = to_string(NewWallX) + " " + to_string(NewWallY) + " H";
                    WallDatas = {NewWallX, NewWallY, "H"};
                }
            }
        }
        // Si la prochaine case est celle du bas
        else if (aPlusCourtChemin[iCaseNext] == (aPlusCourtChemin[iCase] + mWidth))
        {
            int NewWallX = aPlusCourtChemin[iCase] % mWidth;
            int NewWallY = aPlusCourtChemin[iCase] / mWidth + 1;
            
            bConstructible = IsConstructibleHorizontal (aWallsBuilt, NewWallX, NewWallY);
            if (bConstructible)
            {
                WallBuilding = to_string(NewWallX) + " " + to_string(NewWallY) + " H";
                WallDatas = {NewWallX, NewWallY, "H"};
            }
            else
            {
                --NewWallX;
                bConstructible = IsConstructibleHorizontal (aWallsBuilt, NewWallX, NewWallY);
                if (bConstructible)
                {
                    WallBuilding = to_string(NewWallX) + " " + to_string(NewWallY) + " H";
                    WallDatas = {NewWallX, NewWallY, "H"};
                }
            }
        }
        else
        {
            // Cas d'erreur car impossible
        }
        
        if (bConstructible)
        {
            AjoutMurMatriceGrapheLite (WallDatas);
            CalculCheminMinimaux ();
            vector<PlayerDatas>::const_iterator itPlayerDatas = aPlayersDatas.begin ();
            while (itPlayerDatas != aPlayersDatas.end () && bConstructible)
            {
                bConstructible = IsCheminPossiblePlayer (*itPlayerDatas);
                ++itPlayerDatas;
            }
            
            if (false == bConstructible)
            {
                cerr << "Impossible de construire le mur [" << WallDatas.PositionX << "," << WallDatas.PositionY << " " << WallDatas.Orientation << "]. " << "Ce joueur est bloqué " << itPlayerDatas->Direction << endl;
                AjoutMurMatriceGrapheLite (WallDatas, true);
                CalculCheminMinimaux ();
                WallBuilding.clear ();
            }
        }
        
        ++iCase;
        ++iCaseNext;
    }

    return WallBuilding;
}

bool CIA::IsConstructibleVertical (const vector<WallDatas>& aWallsBuilt, const int aNewWallX, const int aNewWallY) const
{
    bool bConstructible = true;
    
    if ((0 <= aNewWallY) && (aNewWallY <= (mHeight - 2)))
    {
        vector<WallDatas>::const_iterator itWallDatas = aWallsBuilt.begin ();
        while ((itWallDatas != aWallsBuilt.end ()) && bConstructible)
        {
            if (itWallDatas->PositionX == aNewWallX)
            {
                if ((0 == itWallDatas->Orientation.compare(string("V"))) && ((itWallDatas->PositionY == aNewWallY) || ((itWallDatas->PositionY - 1) == aNewWallY) || ((itWallDatas->PositionY + 1) == aNewWallY)))
                {
                    bConstructible = false;
                }
            }
            else if (((itWallDatas->PositionX + 1) == aNewWallX) && ((itWallDatas->PositionY - 1) == aNewWallY) && (0 == itWallDatas->Orientation.compare(string("H"))))
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

bool CIA::IsConstructibleHorizontal (const vector<WallDatas>& aWallsBuilt, const int aNewWallX, const int aNewWallY) const
{
    bool bConstructible = true;
    
    if ((0 <= aNewWallX) && (aNewWallX <= (mWidth - 2)))
    {
        vector<WallDatas>::const_iterator itWallDatas = aWallsBuilt.begin ();
        while ((itWallDatas != aWallsBuilt.end ()) && bConstructible)
        {
            if (itWallDatas->PositionY == aNewWallY)
            {
                if ((0 == itWallDatas->Orientation.compare(string("H"))) && ((itWallDatas->PositionX == aNewWallX) || ((itWallDatas->PositionX - 1) == aNewWallX) || ((itWallDatas->PositionX + 1) == aNewWallX)))
                {
                    bConstructible = false;
                }
            }
            else if (((itWallDatas->PositionY + 1) == aNewWallY) && ((itWallDatas->PositionX - 1) == aNewWallX) && (0 == itWallDatas->Orientation.compare(string("V"))))
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


bool CIA::IsCheminPossiblePlayer (const PlayerDatas& aPlayersDatas) const
{
    bool bCheminExiste = false;
    
    if ((aPlayersDatas.PositionX == -1) || (aPlayersDatas.PositionY == -1) || (aPlayersDatas.Direction == -1))
    {
        bCheminExiste = true;
    }
    else
    {
        vector<int> CasesArrivees;
        switch (aPlayersDatas.Direction)
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
            bCheminExiste = IsCheminPossible (aPlayersDatas.PositionX + aPlayersDatas.PositionY * mWidth, CasesArrivees[i]);
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
    int NumCaseMax = mWidth * mHeight - 1;
    
    if (bVertical && (aWallDatas.PositionY != mHeight))
    {
        if ((aWallDatas.PositionX != 0) && (aWallDatas.PositionX != mWidth))
        {
            int NumCase1 = aWallDatas.PositionX - 1 + aWallDatas.PositionY * mWidth;
            int NumCase2 = aWallDatas.PositionX - 1 + (aWallDatas.PositionY + 1) * mWidth;
            
             mMatriceGraph[NumCase1][NumCase1 + 1] *= POIDS_MUR;
             mMatriceGraph[NumCase1 + 1][NumCase1] *= POIDS_MUR;
             mMatriceGraph[NumCase2][NumCase2 + 1] *= POIDS_MUR;
             mMatriceGraph[NumCase2 + 1][NumCase2] *= POIDS_MUR;
        }
    }
    else if ((false == bVertical) && (aWallDatas.PositionX != mWidth))
    {
        if ((aWallDatas.PositionY != 0) &&  (aWallDatas.PositionY != mHeight))
        {
            int NumCase1 = aWallDatas.PositionX + (aWallDatas.PositionY - 1) * mWidth;
            int NumCase2 = aWallDatas.PositionX + 1 + (aWallDatas.PositionY - 1) * mWidth;
            
             mMatriceGraph[NumCase1][NumCase1 + mWidth] *= POIDS_MUR;
             mMatriceGraph[NumCase1 + mWidth][NumCase1] *= POIDS_MUR;
             mMatriceGraph[NumCase2][NumCase2 + mWidth] *= POIDS_MUR;
             mMatriceGraph[NumCase2 + mWidth][NumCase2] *= POIDS_MUR;
        }
    }
}

void CIA::AjoutMurMatriceGrapheLite (const WallDatas& aWallDatas, const bool abDestroy = false)
{
   const bool bVertical = (0 == aWallDatas.Orientation.compare(string("V")));

   if (bVertical && (aWallDatas.PositionY != mHeight))
   {
       if ((aWallDatas.PositionX != 0) && (aWallDatas.PositionX != mWidth))
       {
           int NumCase1 = aWallDatas.PositionX - 1 + aWallDatas.PositionY * mWidth;
           int NumCase2 = aWallDatas.PositionX - 1 + (aWallDatas.PositionY + 1) * mWidth;

           if (abDestroy)
           {
               mMatriceGraph[NumCase1][NumCase1 + 1] /= POIDS_MUR;
               mMatriceGraph[NumCase1 + 1][NumCase1] /= POIDS_MUR;
               mMatriceGraph[NumCase2][NumCase2 + 1] /= POIDS_MUR;
               mMatriceGraph[NumCase2 + 1][NumCase2] /= POIDS_MUR;

               for (int i = -1; i < 3; ++i)
               {
                  int NumCaseSrc  = NumCase1 + (i * mWidth) - 1;
                  int NumCaseDest = NumCase1 + (i * mWidth);
                  if ((0 <=  NumCaseSrc) && (NumCaseSrc <= NumCaseMax) && (0 <=  NumCaseDest) && (NumCaseDest <= NumCaseMax))
                  {
                     mMatriceGraph[NumCaseSrc][NumCaseDest] /= COEF_POIDS_CHEMIN;
                  }
                  NumCaseDest = NumCase1 + (i * mWidth) + 1;
                  NumCaseSrc  = NumCase1 + (i * mWidth) + 2;
                  if ((0 <=  NumCaseSrc) && (NumCaseSrc <= NumCaseMax) && (0 <=  NumCaseDest) && (NumCaseDest <= NumCaseMax))
                  {
                     mMatriceGraph[NumCaseSrc][NumCaseDest] /= COEF_POIDS_CHEMIN;
                  }
               }
               for (int i = -1; i < 1; ++i)
               {
                  int NumCaseSrc  = NumCase1 + (i * mWidth);
                  int NumCaseDest = NumCase1 + ((i + 1) * mWidth);
                  if ((0 <=  NumCaseSrc) && (NumCaseSrc <= NumCaseMax) && (0 <=  NumCaseDest) && (NumCaseDest <= NumCaseMax))
                  {
                     mMatriceGraph[NumCaseSrc][NumCaseDest] /= COEF_POIDS_CHEMIN;
                  }
                  NumCaseSrc  = NumCase1 + (i * mWidth) + 1;
                  NumCaseDest = NumCase1 + ((i + 1) * mWidth) + 1;
                  if ((0 <=  NumCaseSrc) && (NumCaseSrc <= NumCaseMax) && (0 <=  NumCaseDest) && (NumCaseDest <= NumCaseMax))
                  {
                     mMatriceGraph[NumCaseDest][NumCaseSrc] /= COEF_POIDS_CHEMIN;
                  }
                  NumCaseDest = NumCase1 + ((i + 1) * mWidth);
                  NumCaseSrc  = NumCase1 + ((i + 2) * mWidth);
                  if ((0 <=  NumCaseSrc) && (NumCaseSrc <= NumCaseMax) && (0 <=  NumCaseDest) && (NumCaseDest <= NumCaseMax))
                  {
                     mMatriceGraph[NumCaseSrc][NumCaseDest] /= COEF_POIDS_CHEMIN;
                  }
                  NumCaseDest = NumCase1 + ((i + 1) * mWidth) + 1;
                  NumCaseSrc  = NumCase1 + ((i + 2) * mWidth) + 1;
                  if ((0 <=  NumCaseSrc) && (NumCaseSrc <= NumCaseMax) && (0 <=  NumCaseDest) && (NumCaseDest <= NumCaseMax))
                  {
                     mMatriceGraph[NumCaseDest][NumCaseSrc] /= COEF_POIDS_CHEMIN;
                  }
               }
           }
           else
           {
               mMatriceGraph[NumCase1][NumCase1 + 1] *= POIDS_MUR;
               mMatriceGraph[NumCase1 + 1][NumCase1] *= POIDS_MUR;
               mMatriceGraph[NumCase2][NumCase2 + 1] *= POIDS_MUR;
               mMatriceGraph[NumCase2 + 1][NumCase2] *= POIDS_MUR;

               for (int i = -1; i < 3; ++i)
               {
                  int NumCaseSrc  = NumCase1 + (i * mWidth) - 1;
                  int NumCaseDest = NumCase1 + (i * mWidth);
                  if ((0 <=  NumCaseSrc) && (NumCaseSrc <= NumCaseMax) && (0 <=  NumCaseDest) && (NumCaseDest <= NumCaseMax))
                  {
                     mMatriceGraph[NumCaseSrc][NumCaseDest] *= COEF_POIDS_CHEMIN;
                  }
                  NumCaseDest = NumCase1 + (i * mWidth) + 1;
                  NumCaseSrc  = NumCase1 + (i * mWidth) + 2;
                  if ((0 <=  NumCaseSrc) && (NumCaseSrc <= NumCaseMax) && (0 <=  NumCaseDest) && (NumCaseDest <= NumCaseMax))
                  {
                     mMatriceGraph[NumCaseSrc][NumCaseDest] *= COEF_POIDS_CHEMIN;
                  }
               }
               for (int i = -1; i < 1; ++i)
               {
                  int NumCaseSrc  = NumCase1 + (i * mWidth);
                  int NumCaseDest = NumCase1 + ((i + 1) * mWidth);
                  if ((0 <=  NumCaseSrc) && (NumCaseSrc <= NumCaseMax) && (0 <=  NumCaseDest) && (NumCaseDest <= NumCaseMax))
                  {
                     mMatriceGraph[NumCaseSrc][NumCaseDest] *= COEF_POIDS_CHEMIN;
                  }
                  NumCaseSrc  = NumCase1 + (i * mWidth) + 1;
                  NumCaseDest = NumCase1 + ((i + 1) * mWidth) + 1;
                  if ((0 <=  NumCaseSrc) && (NumCaseSrc <= NumCaseMax) && (0 <=  NumCaseDest) && (NumCaseDest <= NumCaseMax))
                  {
                     mMatriceGraph[NumCaseDest][NumCaseSrc] *= COEF_POIDS_CHEMIN;
                  }
                  NumCaseDest = NumCase1 + ((i + 1) * mWidth);
                  NumCaseSrc  = NumCase1 + ((i + 2) * mWidth);
                  if ((0 <=  NumCaseSrc) && (NumCaseSrc <= NumCaseMax) && (0 <=  NumCaseDest) && (NumCaseDest <= NumCaseMax))
                  {
                     mMatriceGraph[NumCaseSrc][NumCaseDest] *= COEF_POIDS_CHEMIN;
                  }
                  NumCaseDest = NumCase1 + ((i + 1) * mWidth) + 1;
                  NumCaseSrc  = NumCase1 + ((i + 2) * mWidth) + 1;
                  if ((0 <=  NumCaseSrc) && (NumCaseSrc <= NumCaseMax) && (0 <=  NumCaseDest) && (NumCaseDest <= NumCaseMax))
                  {
                     mMatriceGraph[NumCaseDest][NumCaseSrc] *= COEF_POIDS_CHEMIN;
                  }
               }
           }
       }
   }
   else if ((false == bVertical) && (aWallDatas.PositionX != mWidth))
   {
       if ((aWallDatas.PositionY != 0) &&  (aWallDatas.PositionY != mHeight))
       {
           int NumCase1 = aWallDatas.PositionX + (aWallDatas.PositionY - 1) * mWidth;
           int NumCase2 = aWallDatas.PositionX + 1 + (aWallDatas.PositionY - 1) * mWidth;

           if (abDestroy)
           {
               mMatriceGraph[NumCase1][NumCase1 + mWidth] /= POIDS_MUR;
               mMatriceGraph[NumCase1 + mWidth][NumCase1] /= POIDS_MUR;
               mMatriceGraph[NumCase2][NumCase2 + mWidth] /= POIDS_MUR;
               mMatriceGraph[NumCase2 + mWidth][NumCase2] /= POIDS_MUR;

                for (int i = -1; i < 3; ++i)
                {
                   int NumCaseSrc  = NumCase1 - mWidth + i;
                   int NumCaseDest = NumCase1 + i;
                   if ((0 <=  NumCaseSrc) && (NumCaseSrc <= NumCaseMax) && (0 <=  NumCaseDest) && (NumCaseDest <= NumCaseMax))
                   {
                      mMatriceGraph[NumCaseSrc][NumCaseDest] /= COEF_POIDS_CHEMIN;
                   }
                   NumCaseSrc  = NumCase1 + (2 * mWidth) + i;
                   NumCaseDest = NumCase1 + mWidth + i;
                   if ((0 <=  NumCaseSrc) && (NumCaseSrc <= NumCaseMax) && (0 <=  NumCaseDest) && (NumCaseDest <= NumCaseMax))
                   {
                      mMatriceGraph[NumCaseSrc][NumCaseDest] /= COEF_POIDS_CHEMIN;
                   }
                }
                for (int i = -1; i < 1; ++i)
                {
                   int NumCaseSrc  = NumCase1 + i;
                   int NumCaseDest = NumCase1 + i + 1;
                   if ((0 <=  NumCaseSrc) && (NumCaseSrc <= NumCaseMax) && (0 <=  NumCaseDest) && (NumCaseDest <= NumCaseMax))
                   {
                      mMatriceGraph[NumCaseSrc][NumCaseDest] /= COEF_POIDS_CHEMIN;
                   }
                   NumCaseSrc  = NumCase1 + mWidth + i;
                   NumCaseDest = NumCase1 + mWidth + i + 1;
                   if ((0 <=  NumCaseSrc) && (NumCaseSrc <= NumCaseMax) && (0 <=  NumCaseDest) && (NumCaseDest <= NumCaseMax))
                   {
                      mMatriceGraph[NumCaseDest][NumCaseSrc] /= COEF_POIDS_CHEMIN;
                   }
                   NumCaseDest = NumCase1 + i + 1;
                   NumCaseSrc  = NumCase1 + i + 2;
                   if ((0 <=  NumCaseSrc) && (NumCaseSrc <= NumCaseMax) && (0 <=  NumCaseDest) && (NumCaseDest <= NumCaseMax))
                   {
                      mMatriceGraph[NumCaseSrc][NumCaseDest] /= COEF_POIDS_CHEMIN;
                   }
                   NumCaseDest = NumCase1 + mWidth + i + 1;
                   NumCaseSrc  = NumCase1 + mWidth + i + 2;
                   if ((0 <=  NumCaseSrc) && (NumCaseSrc <= NumCaseMax) && (0 <=  NumCaseDest) && (NumCaseDest <= NumCaseMax))
                   {
                      mMatriceGraph[NumCaseDest][NumCaseSrc] /= COEF_POIDS_CHEMIN;
                   }
                }
           }
           else
           {
               mMatriceGraph[NumCase1][NumCase1 + mWidth] *= POIDS_MUR;
               mMatriceGraph[NumCase1 + mWidth][NumCase1] *= POIDS_MUR;
               mMatriceGraph[NumCase2][NumCase2 + mWidth] *= POIDS_MUR;
               mMatriceGraph[NumCase2 + mWidth][NumCase2] *= POIDS_MUR;

                for (int i = -1; i < 3; ++i)
                {
                   int NumCaseSrc  = NumCase1 - mWidth + i;
                   int NumCaseDest = NumCase1 + i;
                   if ((0 <=  NumCaseSrc) && (NumCaseSrc <= NumCaseMax) && (0 <=  NumCaseDest) && (NumCaseDest <= NumCaseMax))
                   {
                      mMatriceGraph[NumCaseSrc][NumCaseDest] *= COEF_POIDS_CHEMIN;
                   }
                   NumCaseSrc  = NumCase1 + (2 * mWidth) + i;
                   NumCaseDest = NumCase1 + mWidth + i;
                   if ((0 <=  NumCaseSrc) && (NumCaseSrc <= NumCaseMax) && (0 <=  NumCaseDest) && (NumCaseDest <= NumCaseMax))
                   {
                      mMatriceGraph[NumCaseSrc][NumCaseDest] *= COEF_POIDS_CHEMIN;
                   }
                }
                for (int i = -1; i < 1; ++i)
                {
                   int NumCaseSrc  = NumCase1 + i;
                   int NumCaseDest = NumCase1 + i + 1;
                   if ((0 <=  NumCaseSrc) && (NumCaseSrc <= NumCaseMax) && (0 <=  NumCaseDest) && (NumCaseDest <= NumCaseMax))
                   {
                      mMatriceGraph[NumCaseSrc][NumCaseDest] *= COEF_POIDS_CHEMIN;
                   }
                   NumCaseSrc  = NumCase1 + mWidth + i;
                   NumCaseDest = NumCase1 + mWidth + i + 1;
                   if ((0 <=  NumCaseSrc) && (NumCaseSrc <= NumCaseMax) && (0 <=  NumCaseDest) && (NumCaseDest <= NumCaseMax))
                   {
                      mMatriceGraph[NumCaseDest][NumCaseSrc] *= COEF_POIDS_CHEMIN;
                   }
                   NumCaseDest = NumCase1 + i + 1;
                   NumCaseSrc  = NumCase1 + i + 2;
                   if ((0 <=  NumCaseSrc) && (NumCaseSrc <= NumCaseMax) && (0 <=  NumCaseDest) && (NumCaseDest <= NumCaseMax))
                   {
                      mMatriceGraph[NumCaseSrc][NumCaseDest] *= COEF_POIDS_CHEMIN;
                   }
                   NumCaseDest = NumCase1 + mWidth + i + 1;
                   NumCaseSrc  = NumCase1 + mWidth + i + 2;
                   if ((0 <=  NumCaseSrc) && (NumCaseSrc <= NumCaseMax) && (0 <=  NumCaseDest) && (NumCaseDest <= NumCaseMax))
                   {
                      mMatriceGraph[NumCaseDest][NumCaseSrc] *= COEF_POIDS_CHEMIN;
                   }
                }
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
    
    vector<PlayerDatas> PlayersDatas;
    vector<WallDatas>   WallsDatas;
    
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
            PlayersDatas.push_back ({static_cast<EDirection>(i), x, y, wallsLeft});
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
        int IdPremierPlayer = -1;
        unsigned int DistancePremierPlayer = 999;
        vector< vector<int> > PlayersPCC;
        for (int i = 0; i < playerCount; i++)
        {
            vector<int> PCC;
            IA.CalculPlusCourtCheminPlayer (PlayersDatas[i], PCC);
            PlayersPCC.push_back (PCC);
            
            if (PCC.size() < DistancePremierPlayer)
            {
                IdPremierPlayer = i;
                DistancePremierPlayer = PCC.size();
                
                cerr << "IdPlayer1 = " << i << " (" << DistancePremierPlayer << ")" << endl;
            }
        }
        
        string Action;
        if ((IdPremierPlayer == myId) || (PlayersDatas[myId].WallsLeft == 0) || ((PlayersPCC[myId].size () == (DistancePremierPlayer - Marge)) && (DistancePremierPlayer > 2))
            || (playerCount == 3))
        {
            cerr << "G" << endl;
            Action = IA.GetNextDirection (PlayersPCC[myId]);
        }
        else if (DistancePremierPlayer <= 3)
        //else
        {
            cerr << "B" << endl;
            Action = IA.BuildWall (PlayersDatas, PlayersPCC[IdPremierPlayer], WallsDatas);
            
            if (Action.empty ())
            {
                Action = IA.GetNextDirection (PlayersPCC[myId]);
            }
        }
        else
        {
            Action = IA.GetNextDirection (PlayersPCC[myId]);
        }
        //Action = IA.GetNextDirection (PlayersPCC[myId]);
        cout << Action << endl; // action: LEFT, RIGHT, UP, DOWN or "putX putY putOrientation" to place a wall
    }
}
