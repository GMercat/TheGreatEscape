#include <iostream>
#include <string>
#include <vector>
#include <list>
#include <algorithm>
#include <chrono>

using namespace std;

class Measure
{
public:
   void start() {
      mStartTime = chrono::high_resolution_clock::now();
   }
   double get() {
      auto diffTime = (chrono::high_resolution_clock::now() - mStartTime);
      return chrono::duration<double, std::milli>(diffTime).count();
   }
private:
   chrono::high_resolution_clock::time_point mStartTime; ///< Store the first time measure
};

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

   bool operator== (const Coord& aCoord) const {
      return ((X == aCoord.X) && (Y == aCoord.Y));
   }
};

struct WallDatas
{
   Coord    Pos;
   string   Orientation;

   const string ToString () const {
      return to_string(Pos.X) + " " + to_string(Pos.Y) + " " + Orientation;
   }

   bool operator== (const WallDatas& aWallDatas) const {
      return ((Pos == aWallDatas.Pos) && (Orientation == aWallDatas.Orientation));
   }

   bool IsValid () const {
      return ((Pos.X != -1) && (Pos.Y != -1) && (!Orientation.empty()));
   }
};

struct WallFound
{
   unsigned int   Length;
   double         Score;
   WallDatas      DatasWall;

   WallFound (): Length (0), Score (-100) {}
};

#define POIDS_MUR               9999
#define COEF_POIDS_CHEMIN       1

static bool _bFirstWall = true;
static int  _myId = -1;
static int  _Width = 9;
static int  _Height = 9;

bool IsConstructibleVertical (const vector<WallDatas>& aWallsBuilt, const Coord aNewWall)
{
    bool bConstructible = true;
    if ((0 <= aNewWall.Y) && (aNewWall.Y <= (_Height - 2))) {
        vector<WallDatas>::const_iterator itWallDatas = aWallsBuilt.begin ();
        while ((itWallDatas != aWallsBuilt.end ()) && bConstructible) {
            if (itWallDatas->Pos.X == aNewWall.X) {
                if ((0 == itWallDatas->Orientation.compare(string("V"))) && ((itWallDatas->Pos.Y == aNewWall.Y) || ((itWallDatas->Pos.Y - 1) == aNewWall.Y) || ((itWallDatas->Pos.Y + 1) == aNewWall.Y))) {
                    bConstructible = false;
                }
            }
            else if (((itWallDatas->Pos.X + 1) == aNewWall.X) && ((itWallDatas->Pos.Y - 1) == aNewWall.Y) && (0 == itWallDatas->Orientation.compare(string("H")))) {
                bConstructible = false;
            }
            ++itWallDatas;
        }
    }
    else {
        bConstructible = false;
    }
    return bConstructible;
}

bool IsConstructibleHorizontal (const vector<WallDatas>& aWallsBuilt, const Coord aNewWall)
{
    bool bConstructible = true;
    if ((0 <= aNewWall.X) && (aNewWall.X <= (_Width - 2))) {
        vector<WallDatas>::const_iterator itWallDatas = aWallsBuilt.begin ();
        while ((itWallDatas != aWallsBuilt.end ()) && bConstructible) {
            if (itWallDatas->Pos.Y == aNewWall.Y) {
                if ((0 == itWallDatas->Orientation.compare(string("H"))) && ((itWallDatas->Pos.X == aNewWall.X) || ((itWallDatas->Pos.X - 1) == aNewWall.X) || ((itWallDatas->Pos.X + 1) == aNewWall.X))) {
                    bConstructible = false;
                }
            }
            else if (((itWallDatas->Pos.Y + 1) == aNewWall.Y) && ((itWallDatas->Pos.X - 1) == aNewWall.X) && (0 == itWallDatas->Orientation.compare(string("V")))) {
                bConstructible = false;
            }
            ++itWallDatas;
        }
    }
    else {
        bConstructible = false;
    }
    return bConstructible;
}

class CPlayer
{
public:
   typedef vector<CPlayer> Vector;
   typedef list<CPlayer> List;
public:
   CPlayer (const int& aId,const Coord& aPosition, const int& aWallsLeft):
      mId         (aId),
      mDir  (static_cast<EDirection>(aId)),
      mPos   (aPosition),
      mWallsLeft  (aWallsLeft)
   {};
   ~CPlayer () {};

   inline bool operator== (const CPlayer& aPlayer) {
         return (mId == aPlayer.mId);
   };

   inline bool operator!= (const CPlayer& aPlayer) {
      return (mId != aPlayer.mId);
   };

   inline const int GetId () const {
      return mId;
   };

   inline const EDirection GetDirection () const {
      return mDir;
   };

   inline const Coord GetPosition () const {
      return mPos;
   };

   inline const int GetNbWallsLeft () const {
      return mWallsLeft;
   };

   inline const vector<int>& GetPCC () const {
      return mPCC;
   };

   inline void SetPCC (const vector<int>& aPCC) {
      mPCC = aPCC;
   };

   inline bool IsPositionStartPattern () {
      bool bPosOk = false;
      switch (mDir) {
         case eRight:
            bPosOk = (mPos.X == _Width - 2);
            break;
         case eLeft:
            bPosOk = (mPos.X == 1);
            break;
         case eDown:
            bPosOk = (mPos.Y == _Height - 2);
            break;
         case eUp:
            bPosOk = (mPos.Y == 1);
            break;
         default:
            break;
      }
      return bPosOk;
   }

   inline WallDatas FindWallBeforeMe (const vector<WallDatas>& aWallsDatas) {
      WallDatas WallBeforeMe;
      WallDatas WallSearch1;
      WallDatas WallSearch2;
      switch (mDir) {
         case eRight:
            WallSearch1 = {Coord{mPos.X + 1, mPos.Y}, "V"};
            WallSearch2 = {Coord{mPos.X + 1, mPos.Y - 1}, "V"};
            break;
         case eLeft:
            WallSearch1 = {Coord{mPos.X, mPos.Y}, "V"};
            WallSearch2 = {Coord{mPos.X, mPos.Y - 1}, "V"};
            break;
         case eDown:
            WallSearch1 = {Coord{mPos.X, mPos.Y + 1}, "H"};
            WallSearch2 = {Coord{mPos.X - 1, mPos.Y + 1}, "H"};
            break;
         case eUp:
            WallSearch1 = {Coord{mPos.X, mPos.Y - 1}, "H"};
            WallSearch2 = {Coord{mPos.X - 1, mPos.Y - 1}, "H"};
            break;
         default:
            break;
      }
      bool bWallFound = false;
      vector<WallDatas>::const_iterator ItWall = aWallsDatas.begin ();
      while ((ItWall != aWallsDatas.end ()) && !bWallFound) {
         if (WallSearch1 == (*ItWall)) {
            WallBeforeMe = WallSearch1;
            bWallFound = true;
         }
         else if (WallSearch2 == (*ItWall)) {
            WallBeforeMe = WallSearch2;
            bWallFound = true;
         }
         ++ItWall;
      }
      return WallBeforeMe;
   }

   inline bool StartPattern (const vector<WallDatas>& aWallsDatas, WallDatas& aWall) {
      bool bStart = false;
      aWall = FindWallBeforeMe (aWallsDatas);
      switch (mDir) {
         case eRight:
            bStart = IsConstructibleVertical (aWallsDatas, Coord{aWall.Pos.X - 1, aWall.Pos.Y});
            if (2 <= aWall.Pos.Y) {
               bStart &= IsConstructibleHorizontal (aWallsDatas, Coord{aWall.Pos.X - 1, aWall.Pos.Y - 1});
            }
            if (aWall.Pos.Y <= 5) {
               bStart &= IsConstructibleHorizontal (aWallsDatas, Coord{aWall.Pos.X - 1, aWall.Pos.Y + 3});
            }
            break;
         case eLeft:
            bStart = IsConstructibleVertical (aWallsDatas, Coord{aWall.Pos.X + 1, aWall.Pos.Y});
            if (2 <= aWall.Pos.Y) {
               bStart &= IsConstructibleHorizontal (aWallsDatas, Coord{aWall.Pos.X - 1, aWall.Pos.Y - 1});
            }
            if (aWall.Pos.Y <= 5) {
               bStart &= IsConstructibleHorizontal (aWallsDatas, Coord{aWall.Pos.X - 1, aWall.Pos.Y + 3});
            }
            break;
         case eDown:
            bStart = IsConstructibleHorizontal (aWallsDatas, Coord{aWall.Pos.X, aWall.Pos.Y - 1});
            if (2 <= aWall.Pos.X) {
               bStart &= IsConstructibleVertical (aWallsDatas, Coord{aWall.Pos.X - 1, aWall.Pos.Y - 1});
            }
            if (aWall.Pos.X <= 5) {
               bStart &= IsConstructibleVertical (aWallsDatas, Coord{aWall.Pos.X + 3, aWall.Pos.Y - 1});
            }
            break;
         case eUp:
            bStart = IsConstructibleHorizontal (aWallsDatas, Coord{aWall.Pos.X, aWall.Pos.Y - 1});
            if (2 <= aWall.Pos.X) {
               bStart &= IsConstructibleVertical (aWallsDatas, Coord{aWall.Pos.X - 1, aWall.Pos.Y - 1});
            }
            if (aWall.Pos.X <= 5) {
               bStart &= IsConstructibleVertical (aWallsDatas, Coord{aWall.Pos.X + 3, aWall.Pos.Y - 1});
            }
            break;
         default:
            break;
      }
      return bStart;
   }

   inline unsigned int GetNextStep (const vector<WallDatas>& aWallsDatas, const unsigned int& aCurrentStep, const WallDatas& aWallRef, string& aAction) {
      unsigned int NextStep = 0;
      bool bStepFound = false;
      cerr << "GetNextStep : Step " << aCurrentStep << endl;
      if (aCurrentStep == 0) {
         const WallDatas& WallBehindMe = GetWallBehindMe (aWallRef);
         bool bHasWallBehindMe = WallExist (aWallsDatas, WallBehindMe);
         if (!bHasWallBehindMe &&  WallIsBuilding (aWallsDatas, WallBehindMe)) {
            aAction = WallBehindMe.ToString ();
            NextStep = 1;
            bStepFound = true;
         }
         else if (bHasWallBehindMe) {
            NextStep = GetNextStep (aWallsDatas, NextStep, aWallRef, aAction);
            bStepFound = true;
         }
         else {
            cerr << "GetNextStep : ABORT 1" << endl;
            bStepFound = true;
         }
      }
      if (!bStepFound && (aCurrentStep == 1)) {
         WallDatas WallBuilding = {Coord{-1, -1}, "H"};
         const WallDatas& WallSideLeft  = GetWallSideLeft (aWallRef);
         const WallDatas& WallSideRigth = GetWallSideRigth (aWallRef);
         bool bWallSideLeftExist  = WallExist (aWallsDatas, WallSideLeft);
         bool bWallSideRigthExist = WallExist (aWallsDatas, WallSideRigth);
         if (bWallSideLeftExist && bWallSideRigthExist) {
            cerr << "GetNextStep : ABORT 3" << endl;
            bStepFound = true;
         }
         else if (bWallSideLeftExist && WallPatternLeft2IsBuilding (aWallsDatas, aWallRef, WallBuilding)) {
            aAction = WallBuilding.ToString ();
            NextStep = 2;
            bStepFound = true;
         }
         else if (bWallSideRigthExist && WallPatternRigth2IsBuilding (aWallsDatas, aWallRef, WallBuilding)) {
            aAction = WallBuilding.ToString ();
            NextStep = 2;
            bStepFound = true;
         }
         else if (WallPatternLeftIsBuilding (aWallsDatas, aWallRef, WallBuilding)) {
            aAction = WallBuilding.ToString ();
            NextStep = 2;
            bStepFound = true;
         }
         else if (WallPatternRigthIsBuilding (aWallsDatas, aWallRef, WallBuilding)) {
            aAction = WallBuilding.ToString ();
            NextStep = 2;
            bStepFound = true;
         }
         else {
            cerr << "GetNextStep : ABORT 4" << endl;
            bStepFound = true;
         }
      }
      if (!bStepFound && (aCurrentStep == 2)) {
         WallDatas WallBuilding = {Coord{-1, -1}, "H"};
         if ((mPCC.size () > 2) && WallPatternRigthIsBuilding (aWallsDatas, aWallRef, WallBuilding)) {
            aAction = WallBuilding.ToString ();
            NextStep = 3;
            bStepFound = true;
         }
         else if (mPCC.size () > 2 && WallBuilding.IsValid ()) {
            NextStep = 4;
            bStepFound = true;
         }
         else if (mPCC.size () > 2) {
            cerr << "GetNextStep : ABORT 5" << endl;
            bStepFound = true;
         }
         else {
            NextStep = 4;
            bStepFound = true;
         }
      }
      if (!bStepFound && (aCurrentStep == 3)) {
         NextStep = 4;
         bStepFound = true;
      }
      if (!bStepFound && (aCurrentStep == 4)) {
         NextStep = 4;
         bStepFound = true;
      }

      return NextStep;
   }

   inline WallDatas GetWallBehindMe (const WallDatas& aWallRef) {
      WallDatas WallSearch = aWallRef;
      switch (mDir) {
         case eRight:
            WallSearch.Pos.X = WallSearch.Pos.X - 1;
            break;
         case eLeft:
            WallSearch.Pos.X = WallSearch.Pos.X + 1;
            break;
         case eDown:
            WallSearch.Pos.Y = WallSearch.Pos.Y - 1;
            break;
         case eUp:
            WallSearch.Pos.Y = WallSearch.Pos.Y + 1;
            break;
         default:
            break;
      }
      return WallSearch;
   }

   inline WallDatas GetWallSideLeft (const WallDatas& aWallRef) {
      WallDatas WallSearch = aWallRef;
      switch (mDir) {
         case eRight:
            WallSearch.Pos.Y = WallSearch.Pos.Y - 2;
            break;
         case eLeft:
            WallSearch.Pos.Y = WallSearch.Pos.Y + 2;
            break;
         case eDown:
            WallSearch.Pos.X = WallSearch.Pos.X + 2;
            break;
         case eUp:
            WallSearch.Pos.X = WallSearch.Pos.X - 2;
            break;
         default:
            break;
      }
      return WallSearch;
   }

   inline WallDatas GetWallSideRigth (const WallDatas& aWallRef) {
      WallDatas WallSearch = aWallRef;
      switch (mDir) {
         case eRight:
            WallSearch.Pos.Y = WallSearch.Pos.Y + 2;
            break;
         case eLeft:
            WallSearch.Pos.Y = WallSearch.Pos.Y - 2;
            break;
         case eDown:
            WallSearch.Pos.X = WallSearch.Pos.X - 2;
            break;
         case eUp:
            WallSearch.Pos.X = WallSearch.Pos.X + 2;
            break;
         default:
            break;
      }
      return WallSearch;
   }

   inline bool WallExist (const vector<WallDatas>& aWallsDatas, const WallDatas& aWall) {
      bool bWallFound = false;
      vector<WallDatas>::const_iterator ItWall = aWallsDatas.begin ();
      while ((ItWall != aWallsDatas.end ()) && !bWallFound) {
         if (aWall == (*ItWall)) {
            bWallFound = true;
         }
         ++ItWall;
      }
      return bWallFound;
   }

   inline bool WallIsBuilding (const vector<WallDatas>& aWallsDatas, const WallDatas& aWall) {
      bool bIsBuilding = false;
      if (0 == aWall.Orientation.compare ("H")) {
         bIsBuilding = IsConstructibleHorizontal (aWallsDatas, aWall.Pos);
      }
      else if (0 == aWall.Orientation.compare ("V")) {
         bIsBuilding = IsConstructibleVertical (aWallsDatas, aWall.Pos);
      }
      return bIsBuilding;
   }

   inline bool WallPatternLeft2IsBuilding (const vector<WallDatas>& aWallsDatas, const WallDatas& aWallRef, WallDatas& aWallBuilding) {
      switch (mDir) {
         case eRight:
            aWallBuilding.Pos = Coord {aWallRef.Pos.X - 1, aWallRef.Pos.Y};
            aWallBuilding.Orientation = "H";
            break;
         case eLeft:
            aWallBuilding.Pos = Coord {aWallRef.Pos.X - 1, aWallRef.Pos.Y + 2};
            aWallBuilding.Orientation = "H";
            break;
         case eDown:
            aWallBuilding.Pos = Coord {aWallRef.Pos.X + 2, aWallRef.Pos.Y - 1};
            aWallBuilding.Orientation = "V";
            break;
         case eUp:
            aWallBuilding.Pos = Coord {aWallRef.Pos.X, aWallRef.Pos.Y - 1};
            aWallBuilding.Orientation = "V";
            break;
         default:
            break;
      }
      return WallIsBuilding (aWallsDatas, aWallBuilding);
   }

   inline bool WallPatternRigth2IsBuilding (const vector<WallDatas>& aWallsDatas, const WallDatas& aWallRef, WallDatas& aWallBuilding) {
      switch (mDir) {
         case eRight:
            aWallBuilding.Pos = Coord {aWallRef.Pos.X - 1, aWallRef.Pos.Y + 2};
            aWallBuilding.Orientation = "H";
            break;
         case eLeft:
            aWallBuilding.Pos = Coord {aWallRef.Pos.X - 1, aWallRef.Pos.Y};
            aWallBuilding.Orientation = "H";
            break;
         case eDown:
            aWallBuilding.Pos = Coord {aWallRef.Pos.X, aWallRef.Pos.Y - 1};
            aWallBuilding.Orientation = "V";
            break;
         case eUp:
            aWallBuilding.Pos = Coord {aWallRef.Pos.X + 2, aWallRef.Pos.Y - 1};
            aWallBuilding.Orientation = "V";
            break;
         default:
            break;
      }
      return WallIsBuilding (aWallsDatas, aWallBuilding);
   }

   inline bool WallPatternLeftIsBuilding (const vector<WallDatas>& aWallsDatas, const WallDatas& aWallRef, WallDatas& aWallBuilding) {
      bool bContinue = false;
      switch (mDir) {
         case eRight:
            if (1 < aWallRef.Pos.Y)
            {
               bContinue = true;
               aWallBuilding.Pos = Coord {aWallRef.Pos.X - 1, aWallRef.Pos.Y - 1};
               aWallBuilding.Orientation = "H";
            }
            break;
         case eLeft:
            if (aWallRef.Pos.Y < (_Height - 3))
            {
               bContinue = true;
               aWallBuilding.Pos = Coord {aWallRef.Pos.X - 1, aWallRef.Pos.Y + 3};
               aWallBuilding.Orientation = "H";
            }
            break;
         case eDown:
            if (aWallRef.Pos.X < (_Width - 3))
            {
               bContinue = true;
               aWallBuilding.Pos = Coord {aWallRef.Pos.X + 3, aWallRef.Pos.Y - 1};
               aWallBuilding.Orientation = "V";
            }
            break;
         case eUp:
            if (1 < aWallRef.Pos.X)
            {
               bContinue = true;
               aWallBuilding.Pos = Coord {aWallRef.Pos.X - 1, aWallRef.Pos.Y - 1};
               aWallBuilding.Orientation = "V";
            }
            break;
         default:
            break;
      }
      if (bContinue) {
         bContinue = WallIsBuilding (aWallsDatas, aWallBuilding);
      }
      return bContinue;
   }

   inline bool WallPatternRigthIsBuilding (const vector<WallDatas>& aWallsDatas, const WallDatas& aWallRef, WallDatas& aWallBuilding) {
      bool bContinue = false;
      switch (mDir) {
         case eRight:
            if (aWallRef.Pos.Y < (_Height - 3))
            {
               bContinue = true;
               aWallBuilding.Pos = Coord {aWallRef.Pos.X - 1, aWallRef.Pos.Y + 3};
               aWallBuilding.Orientation = "H";
            }
            break;
         case eLeft:
            if (1 < aWallRef.Pos.Y)
            {
               bContinue = true;
               aWallBuilding.Pos = Coord {aWallRef.Pos.X - 1, aWallRef.Pos.Y - 1};
               aWallBuilding.Orientation = "H";
            }
            break;
         case eDown:
            if (1 < aWallRef.Pos.X)
            {
               bContinue = true;
               aWallBuilding.Pos = Coord {aWallRef.Pos.X - 1, aWallRef.Pos.Y - 1};
               aWallBuilding.Orientation = "V";
            }
            break;
         case eUp:
            if (aWallRef.Pos.X < (_Width - 3))
            {
               bContinue = true;
               aWallBuilding.Pos = Coord {aWallRef.Pos.X + 3, aWallRef.Pos.Y - 1};
               aWallBuilding.Orientation = "V";
            }
            break;
         default:
            break;
      }
      if (bContinue) {
         cerr << "WallPatternRigthIsBuilding " << aWallBuilding.ToString() << endl;
         bContinue = WallIsBuilding (aWallsDatas, aWallBuilding);
      }
      return bContinue;
   }

private:
   const int         mId;
   const EDirection  mDir;
   const Coord       mPos;
   const int         mWallsLeft;

   vector<int>       mPCC;
};

bool ComparePlayer (const CPlayer& aPlayer1, const CPlayer& aPlayer2)
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

class CIA
{
public:
    CIA (const int aWidth, const int aHeight);
    ~CIA();
    
    void    ConstruireMatriceGraphe       ();
    void    AjoutMurMatriceGraphe         (const WallDatas& aWallDatas);
    void    AjoutMurMatriceGrapheLite     (const WallDatas& aWallDatas, const bool abDestroy);
    bool    CalculPCCPlayer   (const CPlayer& aPlayersDatas, vector<int>& aOutPlusCourtChemin);
    void    CalculCheminMinimaux          ();
    
    string  GetNextDirection            (const vector<int>& aPlusCourtChemin);
    string  BuildWallInFrontOfPlayer    (CPlayer::List& aPlayersDatas, CPlayer& aPlayer, const vector<WallDatas>& aWallsBuilt);

    bool TestPositionMur (const CPlayer::List& aPlayersDatas, const WallDatas& aWallDatas);

private:
    bool CalculPCC  (const int aNumCaseDepart, const int aNumCaseArrivee, vector<int>& aOutPlusCourtChemin);
    
    bool BuildVerticalWall    (CPlayer::List& aPlayersDatas, CPlayer& aPlayer, const vector<WallDatas>& aWallsBuilt, const Coord& aCoordWall1, const Coord& aCoordWall2, WallDatas& aWallDatas, unsigned int& aLengthPCC, double& aScore);
    bool BuildHorizontalWall  (CPlayer::List& aPlayersDatas, CPlayer& aPlayer, const vector<WallDatas>& aWallsBuilt, const Coord& aCoordWall1, const Coord& aCoordWall2, WallDatas& aWallDatas, unsigned int& aLengthPCC, double& aScore);

    bool ChercheNewPCCPlayer (CPlayer::List& aPlayersDatas, CPlayer& aPlayer, const WallDatas& aWallDatas, unsigned int& aLengthPCC, double& aScore);
    void SelectionneBestWall (const CPlayer& aPlayerDatas, const bool& abConstructible1, const WallDatas& aWallDatas1, const unsigned int& aLengthPCC1, const double& aScore1, const bool& abConstructible2, const WallDatas& aWallDatas2, const unsigned int& aLengthPCC2, const double& aScore2, WallDatas& aBestWallDatas, unsigned int& aLengthBestPCC, double& aScore);
    
    bool IsCheminPossiblePlayer (const CPlayer& aPlayersDatas) const;
    bool IsCheminPossible       (const int aNumCaseDepart, const int aNumCaseArrivee) const;
private:
    const int mWidth;
    const int mHeight;
    
    int **mMatGraph;
    int **mMatGraphCalcul;
    int **mCheminsMin;
};

CIA::CIA (const int aWidth, const int aHeight):
    mWidth  (aWidth),
    mHeight (aHeight)
{
    const int NbLigneColonne = mWidth * mHeight;
    mMatGraph = new int* [NbLigneColonne];
    mMatGraphCalcul = new int* [NbLigneColonne];
    mCheminsMin = new int* [NbLigneColonne];
    for (int IterLigne = 0; IterLigne < NbLigneColonne ; IterLigne++) {
        mMatGraph[IterLigne] = new int[NbLigneColonne];
        mMatGraphCalcul[IterLigne] = new int[NbLigneColonne];
        mCheminsMin[IterLigne] = new int[NbLigneColonne];
    }
    
    for (int IterLigne= 0; IterLigne < NbLigneColonne; ++IterLigne) {
        for (int IterColonne= 0; IterColonne < NbLigneColonne; ++IterColonne) {
            if (IterLigne == IterColonne) {
                mMatGraph[IterColonne][IterLigne] = 0;
            }
            else {
                mMatGraph[IterColonne][IterLigne] = POIDS_MUR;
            }
        }
    }
}

CIA::~CIA ()
{
    const int NbLigneColonne = mWidth * mHeight;
    for(int IterLigne = 0 ; IterLigne < NbLigneColonne ; ++IterLigne) {
        delete[] mMatGraph[IterLigne];
        delete[] mCheminsMin[IterLigne];
    }
    delete[] mMatGraph;
    delete[] mCheminsMin;
}

void CIA::ConstruireMatriceGraphe ()
{
    int IterLargeur = 0;
    int IterHauteur = 0;
    int NumCaseCourante = -1;
    int NbCaseLargeur = mWidth;
    int NbCaseHauteur = mHeight;
    
    for (; IterLargeur < NbCaseLargeur; ++IterLargeur) {
        for (IterHauteur = 0; IterHauteur < NbCaseHauteur; ++IterHauteur) {
            NumCaseCourante = IterLargeur + (IterHauteur * NbCaseLargeur);
            if (IterLargeur != 0) {
                mMatGraph[NumCaseCourante][(IterLargeur - 1) + IterHauteur * NbCaseLargeur] = 1;
                mMatGraph[(IterLargeur - 1) + IterHauteur * NbCaseLargeur][NumCaseCourante] = 1;
            }
            if (IterLargeur != (NbCaseLargeur - 1)) {
                mMatGraph[NumCaseCourante][(IterLargeur + 1) + IterHauteur * NbCaseLargeur] = 1;
                mMatGraph[(IterLargeur + 1) + IterHauteur * NbCaseLargeur][NumCaseCourante] = 1;
            }
            if (IterHauteur != 0) {
                mMatGraph[NumCaseCourante][IterLargeur + (IterHauteur - 1) * NbCaseLargeur] = 1;
                mMatGraph[IterLargeur + (IterHauteur - 1) * NbCaseLargeur][NumCaseCourante] = 1;
            }
            if (IterHauteur != (NbCaseHauteur - 1)) {
                mMatGraph[NumCaseCourante][IterLargeur + (IterHauteur + 1) * NbCaseLargeur] = 1;
                mMatGraph[IterLargeur + (IterHauteur + 1) * NbCaseLargeur][NumCaseCourante] = 1;
            }
        }
    }
}

void CIA::CalculCheminMinimaux ()
{
    for (int IterLigne = 0; IterLigne < mWidth * mHeight; ++IterLigne) {
        for (int IterColonne = 0; IterColonne < mWidth * mHeight; ++IterColonne) {
            mMatGraphCalcul[IterLigne][IterColonne] = mMatGraph[IterLigne][IterColonne];
            if ((IterLigne != IterColonne) && (mMatGraph[IterLigne][IterColonne] < POIDS_MUR)) {
                mCheminsMin[IterLigne][IterColonne] = IterLigne;
            }
            else {
                mCheminsMin[IterLigne][IterColonne] = POIDS_MUR;
            }
        }
    }
    for (int IterInterm = 0; IterInterm < mWidth * mHeight; ++IterInterm) {
        for (int IterLigne = 0; IterLigne < mWidth * mHeight; ++IterLigne) {
            if (mMatGraphCalcul[IterLigne][IterInterm] < POIDS_MUR) {
                for (int IterColonne = 0; IterColonne < mWidth * mHeight; ++IterColonne) {
                    int Cheminik = mMatGraphCalcul[IterLigne][IterInterm];
                    int Cheminkj = mMatGraphCalcul[IterInterm][IterColonne];
                    if ((Cheminik < POIDS_MUR) && (Cheminkj < POIDS_MUR)) {
                        int Cheminij = Cheminik + Cheminkj;
                        if (Cheminij < mMatGraphCalcul[IterLigne][IterColonne]) {
                            mMatGraphCalcul[IterLigne][IterColonne] = Cheminij;
                            mCheminsMin[IterLigne][IterColonne] = mCheminsMin[IterInterm][IterColonne];
                        }
                    }
                }
            }
        }
    }
}

bool CIA::CalculPCCPlayer (const CPlayer& aPlayersDatas, vector<int>& aOutPlusCourtChemin)
{
   bool bReturn = false;
   vector<int> CasesArrivees;
   switch (aPlayersDatas.GetDirection ()) {
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
   for (unsigned int i=0; i < CasesArrivees.size();++i) {
      vector<int> PCC;
      bReturn = CalculPCC (aPlayersDatas.GetPosition ().X + aPlayersDatas.GetPosition ().Y * mWidth, CasesArrivees[i], PCC);
      if (bReturn && (1 < PCC.size()) && (PCC.size() < DistanceMin)) {
         DistanceMin = PCC.size();
         aOutPlusCourtChemin = PCC;
      }
   }
   return (DistanceMin != 99);
}

bool CIA::CalculPCC (const int aNumCaseDepart, const int aNumCaseArrivee, vector<int>& aOutPlusCourtChemin)
{
    aOutPlusCourtChemin.clear();
    int NumCaseCourante = aNumCaseDepart;
    int NbIterMax = mHeight * mWidth;
    int NbIter = 0;
    aOutPlusCourtChemin.push_back (NumCaseCourante);
    while ((NbIter != NbIterMax) && (NumCaseCourante != aNumCaseArrivee) && (NumCaseCourante < POIDS_MUR)) {
        NumCaseCourante = mCheminsMin[aNumCaseArrivee][NumCaseCourante];
        aOutPlusCourtChemin.push_back (NumCaseCourante);
        ++NbIter;
    }
    return ((NbIter != NbIterMax) && (NumCaseCourante < POIDS_MUR));
}

string CIA::GetNextDirection (const vector<int>& aPlusCourtChemin)
{
    string Direction = "RIGHT";
    if (aPlusCourtChemin.size () >= 2)
    {
        if (aPlusCourtChemin[1] == (aPlusCourtChemin[0] - 1)) {
            Direction = "LEFT";
        }
        else if (aPlusCourtChemin[1] == (aPlusCourtChemin[0] + 1)) {
            Direction = "RIGHT";
        }
        else if (aPlusCourtChemin[1] == (aPlusCourtChemin[0] - mWidth)) {
            Direction = "UP";
        }
        else if (aPlusCourtChemin[1] == (aPlusCourtChemin[0] + mWidth)) {
            Direction = "DOWN";
        }
        else
        {
        }
    }
    return Direction;
}

string CIA::BuildWallInFrontOfPlayer (CPlayer::List& aPlayersDatas, CPlayer& aMe, const vector<WallDatas>& aWallsBuilt)
{
   string WallBuilding;
   CPlayer::List::iterator ItPlayer = aPlayersDatas.begin ();
   while (((*ItPlayer) != aMe) && (WallBuilding.empty ())) {
      CPlayer& Player = *ItPlayer;
      const vector<int> PCCPlayer = Player.GetPCC ();

      WallFound BestWallDatas;
      for (unsigned int iCase = 0; iCase < (PCCPlayer.size () - 1); ++iCase) {
         const unsigned int iCaseNext = iCase + 1;
         bool bConstructible = false;
         WallDatas      WallDatas;
         unsigned int   LengthPCC = 0;
         double         ScorePCC  = 0.0;
         if (PCCPlayer[iCaseNext] == (PCCPlayer[iCase] - 1)) {
            Coord CoordNewWall1 = {PCCPlayer[iCase] % mWidth, PCCPlayer[iCase] / mWidth};
            Coord CoordNewWall2 = {CoordNewWall1.X, CoordNewWall1.Y - 1};

            bConstructible = BuildVerticalWall (aPlayersDatas, Player, aWallsBuilt, CoordNewWall1, CoordNewWall2, WallDatas, LengthPCC, ScorePCC);
         }
         else if (PCCPlayer[iCaseNext] == (PCCPlayer[iCase] + 1)) {
            Coord CoordNewWall1 = {PCCPlayer[iCase] % mWidth + 1, PCCPlayer[iCase] / mWidth};
            Coord CoordNewWall2 = {CoordNewWall1.X, CoordNewWall1.Y - 1};

            bConstructible = BuildVerticalWall (aPlayersDatas, Player, aWallsBuilt, CoordNewWall1, CoordNewWall2, WallDatas, LengthPCC, ScorePCC);
         }
         else if (PCCPlayer[iCaseNext] == (PCCPlayer[iCase] - mWidth)) {
            Coord CoordNewWall1 = {PCCPlayer[iCase] % mWidth, PCCPlayer[iCase] / mWidth};
            Coord CoordNewWall2 = {CoordNewWall1.X - 1, CoordNewWall1.Y};

            bConstructible = BuildHorizontalWall (aPlayersDatas, Player, aWallsBuilt, CoordNewWall1, CoordNewWall2, WallDatas, LengthPCC, ScorePCC);
         }
         else if (PCCPlayer[iCaseNext] == (PCCPlayer[iCase] + mWidth)) {
            Coord CoordNewWall1 = {PCCPlayer[iCase] % mWidth, PCCPlayer[iCase] / mWidth + 1};
            Coord CoordNewWall2 = {CoordNewWall1.X - 1, CoordNewWall1.Y};

            bConstructible = BuildHorizontalWall (aPlayersDatas, Player, aWallsBuilt, CoordNewWall1, CoordNewWall2, WallDatas, LengthPCC, ScorePCC);
         }
         if (bConstructible) {
            cerr << "BestWallDatas.Score=" << BestWallDatas.Score << " ScorePCC=" << ScorePCC << endl;
            //if (BestWallDatas.Length < LengthPCC) // TODO <=
            if (BestWallDatas.Score < ScorePCC)
            {
               BestWallDatas.Length    = LengthPCC;
               BestWallDatas.Score     = ScorePCC;
               BestWallDatas.DatasWall = WallDatas;

               WallBuilding = BestWallDatas.DatasWall.ToString ();
            }
         }
      }
      ++ItPlayer;
   }
   return WallBuilding;
}

bool CIA::BuildVerticalWall (CPlayer::List& aPlayersDatas, CPlayer& aPlayer, const vector<WallDatas>& aWallsBuilt, const Coord& aCoordWall1, const Coord& aCoordWall2, WallDatas& aWallDatas, unsigned int& aLengthPCC, double& aScore)
{
   bool bConstructible1 = IsConstructibleVertical (aWallsBuilt, aCoordWall1);
   bool bConstructible2 = IsConstructibleVertical (aWallsBuilt, aCoordWall2);
   WallDatas WallDatas1 = {aCoordWall1, "V"};
   WallDatas WallDatas2 = {aCoordWall2, "V"};
   unsigned int LengthPCC1 = 99;
   unsigned int LengthPCC2 = 99;
   double ScorePCC1 = 0;
   double ScorePCC2 = 0;
   if (bConstructible1 && bConstructible2) {
      bConstructible1 = ChercheNewPCCPlayer (aPlayersDatas, aPlayer, WallDatas1, LengthPCC1, ScorePCC1);
      bConstructible2 = ChercheNewPCCPlayer (aPlayersDatas, aPlayer, WallDatas2, LengthPCC2, ScorePCC2);
      SelectionneBestWall (aPlayer, bConstructible1, WallDatas1, LengthPCC1, ScorePCC1, bConstructible2, WallDatas2, LengthPCC2, ScorePCC2, aWallDatas, aLengthPCC, aScore);
   }
   else if (bConstructible1) {
      bConstructible1 = ChercheNewPCCPlayer (aPlayersDatas, aPlayer, WallDatas1, LengthPCC1, ScorePCC1);
      if (bConstructible1) {
         aWallDatas  = WallDatas1;
         aLengthPCC  = LengthPCC1;
         aScore      = ScorePCC1;
      }
   }
   else if (bConstructible2) {
      bConstructible2 = ChercheNewPCCPlayer (aPlayersDatas, aPlayer, WallDatas2, LengthPCC2, ScorePCC2);
      if (bConstructible2) {
         aWallDatas  = WallDatas2;
         aLengthPCC  = LengthPCC2;
         aScore      = ScorePCC2;
      }
   }
   return bConstructible1 || bConstructible2;
}

bool CIA::BuildHorizontalWall (CPlayer::List& aPlayersDatas, CPlayer& aPlayer, const vector<WallDatas>& aWallsBuilt, const Coord& aCoordWall1, const Coord& aCoordWall2, WallDatas& aWallDatas, unsigned int& aLengthPCC, double& aScore)
{
   bool bConstructible1 = IsConstructibleHorizontal (aWallsBuilt, aCoordWall1);
   bool bConstructible2 = IsConstructibleHorizontal (aWallsBuilt, aCoordWall2);
   WallDatas WallDatas1 = {aCoordWall1, "H"};
   WallDatas WallDatas2 = {aCoordWall2, "H"};
   unsigned int LengthPCC1 = 99;
   unsigned int LengthPCC2 = 99;
   double ScorePCC1 = 0;
   double ScorePCC2 = 0;
   if (bConstructible1 && bConstructible2) {
      bConstructible1 = ChercheNewPCCPlayer (aPlayersDatas, aPlayer, WallDatas1, LengthPCC1, ScorePCC1);
      bConstructible2 = ChercheNewPCCPlayer (aPlayersDatas, aPlayer, WallDatas2, LengthPCC2, ScorePCC2);
      SelectionneBestWall (aPlayer, bConstructible1, WallDatas1, LengthPCC1, ScorePCC1, bConstructible2, WallDatas2, LengthPCC2, ScorePCC2, aWallDatas, aLengthPCC, aScore);
   }
   else if (bConstructible1) {
      bConstructible1 = ChercheNewPCCPlayer (aPlayersDatas, aPlayer, WallDatas1, LengthPCC1, ScorePCC1);
      if (bConstructible1) {
         aWallDatas  = WallDatas1;
         aLengthPCC  = LengthPCC1;
         aScore      = ScorePCC1;
      }
   }
   else if (bConstructible2) {
      bConstructible2 = ChercheNewPCCPlayer (aPlayersDatas, aPlayer, WallDatas2, LengthPCC2, ScorePCC2);
      if (bConstructible2) {
         aWallDatas  = WallDatas2;
         aLengthPCC  = LengthPCC2;
         aScore      = ScorePCC2;
      }
   }
   return bConstructible1 || bConstructible2;
}

bool CIA::TestPositionMur (const CPlayer::List& aPlayersDatas, const WallDatas& aWallDatas)
{
   bool bConstructible = true;
   AjoutMurMatriceGrapheLite (aWallDatas, false);
   CalculCheminMinimaux ();
   CPlayer::List::const_iterator itPlayerDatas = aPlayersDatas.begin ();
   while (itPlayerDatas != aPlayersDatas.end () && bConstructible) {
      vector<int> PCC;
      bConstructible = CalculPCCPlayer (*itPlayerDatas, PCC);
      ++itPlayerDatas;
   }
   AjoutMurMatriceGrapheLite (aWallDatas, true);
   CalculCheminMinimaux ();
   return bConstructible;
}

bool CIA::ChercheNewPCCPlayer (CPlayer::List& aPlayersDatas, CPlayer& aPlayer, const WallDatas& aWallDatas, unsigned int& aLengthPCC, double& aScore)
{
   bool bConstructible = true;
   AjoutMurMatriceGrapheLite (aWallDatas, false);
   CalculCheminMinimaux ();
   CPlayer::List::iterator itPlayerDatas = aPlayersDatas.begin ();
   while (itPlayerDatas != aPlayersDatas.end () && bConstructible) {
      vector<int> PCC;
      bConstructible = CalculPCCPlayer (*itPlayerDatas, PCC);
      if (bConstructible) {
         const int Length = PCC.size ();
         if ((*itPlayerDatas).GetId () == _myId) {
            aScore += (100.0 / Length);
         }
         else if ((*itPlayerDatas) == aPlayer) {
            aScore -= (100.0 / Length);
            aLengthPCC = PCC.size ();
         }
      }
      ++itPlayerDatas;
   }
   AjoutMurMatriceGrapheLite (aWallDatas, true);
   CalculCheminMinimaux ();
   return bConstructible;
}

void CIA::SelectionneBestWall (const CPlayer& aPlayerDatas,
const bool& abConstructible1,
const WallDatas& aWallDatas1,
const unsigned int& aLengthPCC1,
const double& aScore1,
const bool& abConstructible2,
const WallDatas& aWallDatas2,
const unsigned int& aLengthPCC2,
const double& aScore2,
WallDatas& aBestWallDatas,
unsigned int& aLengthBestPCC,
double& aScore)
{
   if (abConstructible1 && abConstructible2) {
      if (_bFirstWall) {
         if ((aPlayerDatas.GetDirection () == eRight) || (aPlayerDatas.GetDirection () == eLeft)) {
            if (aPlayerDatas.GetPosition ().Y < ((mHeight - 1) / 2)) {
               if (0 == (aWallDatas1.Pos.Y % 2)) {
                  aBestWallDatas = aWallDatas1;
                  aLengthBestPCC = aLengthPCC1;
                  aScore         = aScore1;
               }
               else {
                  aBestWallDatas = aWallDatas2;
                  aLengthBestPCC = aLengthPCC2;
                  aScore         = aScore2;
               }
            }
            else {
               if (0 == (aWallDatas1.Pos.Y % 2)) {
                  aBestWallDatas = aWallDatas2;
                  aLengthBestPCC = aLengthPCC2;
                  aScore         = aScore2;
               }
               else {
                  aBestWallDatas = aWallDatas1;
                  aLengthBestPCC = aLengthPCC1;
                  aScore         = aScore1;
               }
            }
         }
         else {
            if (aPlayerDatas.GetPosition ().X < ((mWidth - 1) / 2)) {
               if (0 == (aWallDatas1.Pos.X % 2)) {
                  aBestWallDatas = aWallDatas1;
                  aLengthBestPCC = aLengthPCC1;
                  aScore         = aScore1;
               }
               else {
                  aBestWallDatas = aWallDatas2;
                  aLengthBestPCC = aLengthPCC2;
                  aScore         = aScore2;
               }
            }
            else {
               if (0 == (aWallDatas1.Pos.X % 2)) {
                  aBestWallDatas = aWallDatas2;
                  aLengthBestPCC = aLengthPCC2;
                  aScore         = aScore2;
               }
               else {
                  aBestWallDatas = aWallDatas1;
                  aLengthBestPCC = aLengthPCC1;
                  aScore         = aScore1;
               }
            }
         }
      }
      else {
         aBestWallDatas = aWallDatas1;
         aLengthBestPCC = aLengthPCC1;
         aScore         = aScore1;
         //if (aLengthPCC1 < aLengthPCC2) // TODO
         if (aScore1 < aScore2) {
            aBestWallDatas = aWallDatas2;
            aLengthBestPCC = aLengthPCC2;
            aScore         = aScore2;
         }
      }
   }
   else if (abConstructible1) {
      aBestWallDatas = aWallDatas1;
      aLengthBestPCC = aLengthPCC1;
      aScore         = aScore1;
   }
   else if (abConstructible2) {
      aBestWallDatas = aWallDatas2;
      aLengthBestPCC = aLengthPCC2;
      aScore         = aScore2;
   }
}

bool CIA::IsCheminPossiblePlayer (const CPlayer& aPlayersDatas) const
{
    bool bCheminExiste = false;
    if ((aPlayersDatas.GetPosition ().X == -1) || (aPlayersDatas.GetPosition ().Y == -1) || (aPlayersDatas.GetDirection () == -1)) {
        bCheminExiste = true;
    }
    else {
        vector<int> CasesArrivees;
        switch (aPlayersDatas.GetDirection ()) {
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
    while ((NbIter != NbIterMax) && (NumCaseCourante != aNumCaseArrivee) && (NumCaseCourante < POIDS_MUR)) {
        NumCaseCourante = mCheminsMin[aNumCaseArrivee][NumCaseCourante];
        ++NbIter;
    }
    return ((NbIter != NbIterMax) && (NumCaseCourante < POIDS_MUR));
}

void CIA::AjoutMurMatriceGraphe (const WallDatas& aWallDatas)
{
   const bool bVertical = (0 == aWallDatas.Orientation.compare(string("V")));
   if (bVertical && (aWallDatas.Pos.Y != mHeight)) {
      if ((aWallDatas.Pos.X != 0) && (aWallDatas.Pos.X != mWidth)) {
         int NumCase1 = aWallDatas.Pos.X - 1 + aWallDatas.Pos.Y * mWidth;
         int NumCase2 = aWallDatas.Pos.X - 1 + (aWallDatas.Pos.Y + 1) * mWidth;
         mMatGraph[NumCase1][NumCase1 + 1] *= POIDS_MUR;
         mMatGraph[NumCase1 + 1][NumCase1] *= POIDS_MUR;
         mMatGraph[NumCase2][NumCase2 + 1] *= POIDS_MUR;
         mMatGraph[NumCase2 + 1][NumCase2] *= POIDS_MUR;
      }
   }
   else if ((false == bVertical) && (aWallDatas.Pos.X != mWidth)) {
      if ((aWallDatas.Pos.Y != 0) &&  (aWallDatas.Pos.Y != mHeight)) {
         int NumCase1 = aWallDatas.Pos.X + (aWallDatas.Pos.Y - 1) * mWidth;
         int NumCase2 = aWallDatas.Pos.X + 1 + (aWallDatas.Pos.Y - 1) * mWidth;
         mMatGraph[NumCase1][NumCase1 + mWidth] *= POIDS_MUR;
         mMatGraph[NumCase1 + mWidth][NumCase1] *= POIDS_MUR;
         mMatGraph[NumCase2][NumCase2 + mWidth] *= POIDS_MUR;
         mMatGraph[NumCase2 + mWidth][NumCase2] *= POIDS_MUR;
        }
    }
}

void CIA::AjoutMurMatriceGrapheLite (const WallDatas& aWallDatas, const bool abDestroy)
{
   const bool bVertical = (0 == aWallDatas.Orientation.compare(string("V")));
   if (bVertical && (aWallDatas.Pos.Y != mHeight)) {
      if ((aWallDatas.Pos.X != 0) && (aWallDatas.Pos.X != mWidth)) {
         int NumCase1 = aWallDatas.Pos.X - 1 + aWallDatas.Pos.Y * mWidth;
         int NumCase2 = aWallDatas.Pos.X - 1 + (aWallDatas.Pos.Y + 1) * mWidth;
         if (abDestroy) {
            mMatGraph[NumCase1][NumCase1 + 1] /= POIDS_MUR;
            mMatGraph[NumCase1 + 1][NumCase1] /= POIDS_MUR;
            mMatGraph[NumCase2][NumCase2 + 1] /= POIDS_MUR;
            mMatGraph[NumCase2 + 1][NumCase2] /= POIDS_MUR;
         }
         else {
            mMatGraph[NumCase1][NumCase1 + 1] *= POIDS_MUR;
            mMatGraph[NumCase1 + 1][NumCase1] *= POIDS_MUR;
            mMatGraph[NumCase2][NumCase2 + 1] *= POIDS_MUR;
            mMatGraph[NumCase2 + 1][NumCase2] *= POIDS_MUR;
         }
      }
   }
   else if ((false == bVertical) && (aWallDatas.Pos.X != mWidth)) {
      if ((aWallDatas.Pos.Y != 0) &&  (aWallDatas.Pos.Y != mHeight)) {
         int NumCase1 = aWallDatas.Pos.X + (aWallDatas.Pos.Y - 1) * mWidth;
         int NumCase2 = aWallDatas.Pos.X + 1 + (aWallDatas.Pos.Y - 1) * mWidth;
         if (abDestroy) {
            mMatGraph[NumCase1][NumCase1 + mWidth] /= POIDS_MUR;
            mMatGraph[NumCase1 + mWidth][NumCase1] /= POIDS_MUR;
            mMatGraph[NumCase2][NumCase2 + mWidth] /= POIDS_MUR;
            mMatGraph[NumCase2 + mWidth][NumCase2] /= POIDS_MUR;
         }
         else {
            mMatGraph[NumCase1][NumCase1 + mWidth] *= POIDS_MUR;
            mMatGraph[NumCase1 + mWidth][NumCase1] *= POIDS_MUR;
            mMatGraph[NumCase2][NumCase2 + mWidth] *= POIDS_MUR;
            mMatGraph[NumCase2 + mWidth][NumCase2] *= POIDS_MUR;
         }
      }
   }
}

int main()
{
   int playerCount;
   cin >> _Width >> _Height >> playerCount >> _myId; cin.ignore();
   CPlayer::List     PlayersDatasOrdonnes;
   vector<WallDatas>      WallsDatas;
   bool                   bBuildingOn = false;
   bool           bModePatternOn = true;
   bool           bPatternStarted = false;
   unsigned int   CurrentStep = 0;
   WallDatas      WallDatasRef;
   Measure measure;
   CIA IA (_Width, _Height);
   while (1) {
      IA.ConstruireMatriceGraphe ();
      unsigned int Marge = playerCount - 1;
      Marge = 0;
      PlayersDatasOrdonnes.clear();
      WallsDatas.clear ();
      for (int i = 0; i < playerCount; i++) {
         int x;
         int y;
         int wallsLeft; // number of walls available for the player
         cin >> x >> y >> wallsLeft; cin.ignore();
         cerr << x << " " << y << " " << wallsLeft << endl;
         if ((x != -1) && (y != -1) && (wallsLeft != -1)) {
            PlayersDatasOrdonnes.push_back (CPlayer(static_cast<EDirection>(i), Coord{x, y}, wallsLeft));
         }
      }
      int wallCount;
      cin >> wallCount; cin.ignore();
      for (int i = 0; i < wallCount; i++) {
         int wallX;
         int wallY;
         string wallOrientation;
         cin >> wallX >> wallY >> wallOrientation; cin.ignore();
         WallsDatas.push_back ({wallX, wallY, wallOrientation});
         IA.AjoutMurMatriceGraphe ({wallX, wallY, wallOrientation});
      }
      measure.start ();
      IA.CalculCheminMinimaux ();
      bool  bAvance = true;
      bool  bPattern = false;
      string Action;
      CPlayer::List::iterator ItPlayerOrdonnes;
      for (ItPlayerOrdonnes = PlayersDatasOrdonnes.begin ();
           ItPlayerOrdonnes != PlayersDatasOrdonnes.end ();
           ++ItPlayerOrdonnes) {
         vector<int> PCC;
         IA.CalculPCCPlayer (*ItPlayerOrdonnes, PCC);
         (*ItPlayerOrdonnes).SetPCC (PCC);
      }
      PlayersDatasOrdonnes.sort (ComparePlayer);
      CPlayer::List::iterator Me;
      for (ItPlayerOrdonnes = PlayersDatasOrdonnes.begin ();
           ItPlayerOrdonnes != PlayersDatasOrdonnes.end ();
           ++ItPlayerOrdonnes) {
         if (ItPlayerOrdonnes->GetId() == _myId) {
            Me = ItPlayerOrdonnes;
         }
      }

      cerr << "P " << bPattern << " PS " << bPatternStarted << endl;;
      if (PlayersDatasOrdonnes.size () == 2) {
         bAvance =  ((*Me).GetNbWallsLeft () == 0);
         bAvance |= ((*Me) == PlayersDatasOrdonnes.front ());
         bAvance |= ((*Me).GetPCC ().size () == (PlayersDatasOrdonnes.front ().GetPCC ().size () - Marge));
         bAvance |= (((*Me) != PlayersDatasOrdonnes.front ()) && ((PlayersDatasOrdonnes.front ().GetPCC ().size () > 3) && !bBuildingOn));
         if (bModePatternOn) {
            CPlayer::List::iterator OtherPlayer = PlayersDatasOrdonnes.begin ();
            if (OtherPlayer == Me) {
               ++OtherPlayer;
            }
            if (((*Me).GetPCC ().size () > 2)) {
               if (!bPatternStarted && (*Me).IsPositionStartPattern ()) {
                  cerr << "1" << endl;
                  if ((*OtherPlayer).GetPCC ().size () > 2) {
                     cerr << "2" << endl;
                     if ((*Me).StartPattern (WallsDatas, WallDatasRef)) {
                        cerr << "3" << endl;
                        CurrentStep = 0;
                        bPattern = true;
                        bPatternStarted = true;
                     }
                  }
                  else {
                     // Il faut que l'ennemi soit éloigné
                     bAvance = false;
                  }
               }
               if (bPatternStarted) {
                  cerr << (*OtherPlayer).GetId () << " : " << (*OtherPlayer).GetPCC ().size () << endl;
                  if ((*OtherPlayer).GetPCC ().size () > 3) {
                     CurrentStep = (*Me).GetNextStep (WallsDatas, CurrentStep, WallDatasRef, Action);
                     if (CurrentStep == 0) {
                        bPattern = false;
                        bPatternStarted = false;
                     }
                     else if (CurrentStep == 4) {
                        bPattern = false;
                     }
                     else {
                        bPattern = true;
                     }
                  }
                  else {
                     if ((*Me) == PlayersDatasOrdonnes.front ()) {
                        PlayersDatasOrdonnes.splice (PlayersDatasOrdonnes.end (), PlayersDatasOrdonnes, Me);
                     }
                     bPattern = false;
                     bAvance = false;
                  }
               }
            }
            else if (((*Me) != PlayersDatasOrdonnes.front ()) && ((*OtherPlayer).GetPCC ().size () < 5)) {
               bAvance = false;
            }
         }
      }
      else if (PlayersDatasOrdonnes.size () == 3) {
         bAvance =  ((*Me).GetNbWallsLeft () == 0);
         bAvance |= ((*Me) != PlayersDatasOrdonnes.back ());
         bAvance |= (((*Me) != PlayersDatasOrdonnes.back ()) && ((*Me).GetPCC ().size () == (PlayersDatasOrdonnes.back ().GetPCC ().size () - Marge)));
         bAvance |= (((*Me) != PlayersDatasOrdonnes.front ()) && ((PlayersDatasOrdonnes.front ().GetPCC ().size () > 2) && !bBuildingOn));
         bAvance |= (((*Me) != PlayersDatasOrdonnes.back ()) && ((*Me) != PlayersDatasOrdonnes.front ()) && (PlayersDatasOrdonnes.back ().GetNbWallsLeft () != 0));
      }

      if (bPattern) {
         cerr << "P" << endl;
      }
      else if (bAvance) {
         cerr << "G" << endl;
         Action = IA.GetNextDirection ((*Me).GetPCC ());
      }
      else {
         cerr << "B" << endl;
         bBuildingOn = true;

         Action = IA.BuildWallInFrontOfPlayer (PlayersDatasOrdonnes, *Me, WallsDatas);

         if (Action.empty ()) {
            Action = IA.GetNextDirection ((*Me).GetPCC ());
         }
         else {
            _bFirstWall = false;
         }
      }
      cout << Action << " " << measure.get() << endl;
   }
}
