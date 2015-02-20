#include <iostream>
#include <string>
#include <vector>
#include <list>
#include <algorithm>
#include <chrono>

using namespace std;

/// Time measure using C++11 std::chrono
class Measure
{
public:
   /// Start a time measure
   void start() {
      // std::chrono::steady_clock would be more stable, but does not exist in Travis CI GCC 4.6
      mStartTime = chrono::high_resolution_clock::now();
   }
   /// Get time elapsed since first time measure
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
   Coord    Position;
   string   Orientation;

   const string ToString (void) const {
      return to_string(Position.X) + " " + to_string(Position.Y) + " " + Orientation;
   }

   bool operator== (const WallDatas& aWallDatas) const {
      return ((Position == aWallDatas.Position) && (Orientation == aWallDatas.Orientation));
   }

   bool IsValid (void) const {
      return ((Position.X != -1) && (Position.Y != -1) && (!Orientation.empty()));
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
static int  _myId = -1; // id of my player (0 = 1st player, 1 = 2nd player, ...)
static int  _Width = 9;
static int  _Height = 9;

bool IsConstructibleVertical (const vector<WallDatas>& aWallsBuilt, const Coord aNewWall)
{
    bool bConstructible = true;

    if ((0 <= aNewWall.Y) && (aNewWall.Y <= (_Height - 2)))
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

bool IsConstructibleHorizontal (const vector<WallDatas>& aWallsBuilt, const Coord aNewWall)
{
    bool bConstructible = true;

    if ((0 <= aNewWall.X) && (aNewWall.X <= (_Width - 2)))
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

   inline bool IsPositionStartPattern () {
      bool bPositionPatternOk = false;
      switch (mDirection) {
         case eRight:
            bPositionPatternOk = (mPosition.X == _Width - 2);
            break;
         case eLeft:
            bPositionPatternOk = (mPosition.X == 1);
            break;
         case eDown:
            bPositionPatternOk = (mPosition.Y == _Height - 2);
            break;
         case eUp:
            bPositionPatternOk = (mPosition.Y == 1);
            break;
         default:
            break;
      }
      return bPositionPatternOk;
   }

   inline WallDatas FindWallBeforeMe (const vector<WallDatas>& aWallsDatas) {
      WallDatas WallBeforeMe;
      WallDatas WallSearch1;
      WallDatas WallSearch2;
      switch (mDirection) {
         case eRight:
            WallSearch1 = {Coord{mPosition.X + 1, mPosition.Y}, "H"};
            WallSearch2 = {Coord{mPosition.X + 1, mPosition.Y - 1}, "H"};
            break;
         case eLeft:
            WallSearch1 = {Coord{mPosition.X - 1, mPosition.Y}, "H"};
            WallSearch2 = {Coord{mPosition.X - 1, mPosition.Y - 1}, "H"};
            break;
         case eDown:
            WallSearch1 = {Coord{mPosition.X, mPosition.Y + 1}, "V"};
            WallSearch2 = {Coord{mPosition.X - 1, mPosition.Y + 1}, "V"};
            break;
         case eUp:
            WallSearch1 = {Coord{mPosition.X, mPosition.Y - 1}, "V"};
            WallSearch2 = {Coord{mPosition.X - 1, mPosition.Y - 1}, "V"};
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

   inline bool StartPattern (const vector<WallDatas>& aWallsDatas, WallDatas& aWallBeforeMe) {
      bool bStart = false;
      aWallBeforeMe = FindWallBeforeMe (aWallsDatas);
      switch (mDirection) {
         case eRight:
            bStart = IsConstructibleVertical (aWallsDatas, Coord{aWallBeforeMe.Position.X - 1, aWallBeforeMe.Position.Y});
            if (2 <= aWallBeforeMe.Position.Y) {
               bStart &= IsConstructibleHorizontal (aWallsDatas, Coord{aWallBeforeMe.Position.X - 1, aWallBeforeMe.Position.Y - 1});
            }
            if (aWallBeforeMe.Position.Y <= 5) {
               bStart &= IsConstructibleHorizontal (aWallsDatas, Coord{aWallBeforeMe.Position.X - 1, aWallBeforeMe.Position.Y + 3});
            }
            break;
         case eLeft:
            bStart = IsConstructibleVertical (aWallsDatas, Coord{aWallBeforeMe.Position.X + 1, aWallBeforeMe.Position.Y});
            if (2 <= aWallBeforeMe.Position.Y) {
               bStart &= IsConstructibleHorizontal (aWallsDatas, Coord{aWallBeforeMe.Position.X - 1, aWallBeforeMe.Position.Y - 1});
            }
            if (aWallBeforeMe.Position.Y <= 5) {
               bStart &= IsConstructibleHorizontal (aWallsDatas, Coord{aWallBeforeMe.Position.X - 1, aWallBeforeMe.Position.Y + 3});
            }
            break;
         case eDown:
            bStart = IsConstructibleHorizontal (aWallsDatas, Coord{aWallBeforeMe.Position.X, aWallBeforeMe.Position.Y - 1});
            if (2 <= aWallBeforeMe.Position.X) {
               bStart &= IsConstructibleVertical (aWallsDatas, Coord{aWallBeforeMe.Position.X - 1, aWallBeforeMe.Position.Y - 1});
            }
            if (aWallBeforeMe.Position.X <= 5) {
               bStart &= IsConstructibleVertical (aWallsDatas, Coord{aWallBeforeMe.Position.X + 3, aWallBeforeMe.Position.Y - 1});
            }
            break;
         case eUp:
            bStart = IsConstructibleHorizontal (aWallsDatas, Coord{aWallBeforeMe.Position.X, aWallBeforeMe.Position.Y - 1});
            if (2 <= aWallBeforeMe.Position.X) {
               bStart &= IsConstructibleVertical (aWallsDatas, Coord{aWallBeforeMe.Position.X - 1, aWallBeforeMe.Position.Y - 1});
            }
            if (aWallBeforeMe.Position.X <= 5) {
               bStart &= IsConstructibleVertical (aWallsDatas, Coord{aWallBeforeMe.Position.X + 3, aWallBeforeMe.Position.Y - 1});
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
            // ABORT
            cerr << "GetNextStep : ABORT 1" << endl;
            bStepFound = true;
         }
      }
      else {
         // ABORT
         cerr << "GetNextStep : ABORT 2" << endl;
         bStepFound = true;
      }
      if (!bStepFound && (aCurrentStep == 1)) {
         WallDatas WallBuilding = {Coord{-1, -1}, "H"};
         const WallDatas& WallSideLeft  = GetWallSideLeft (aWallRef);
         const WallDatas& WallSideRigth = GetWallSideRigth (aWallRef);
         bool bWallSideLeftExist  = WallExist (aWallsDatas, WallSideLeft);
         bool bWallSideRigthExist = WallExist (aWallsDatas, WallSideRigth);
         if (bWallSideLeftExist && bWallSideRigthExist) {
            // ABORT
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
            // ABORT
            cerr << "GetNextStep : ABORT 4" << endl;
            bStepFound = true;
         }
      }
      if (!bStepFound && (aCurrentStep == 2)) {
         WallDatas WallBuilding = {Coord{-1, -1}, "H"};
         if ((mPCC.size () > 3) && WallPatternRigthIsBuilding (aWallsDatas, aWallRef, WallBuilding)) {
            aAction = WallBuilding.ToString ();
            NextStep = 3;
            bStepFound = true;
         }
         else if (mPCC.size () > 3 && WallBuilding.IsValid ()) {
            NextStep = 4;
            bStepFound = true;
         }
         else if (mPCC.size () > 3) {
            // ABORT
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
      switch (mDirection) {
         case eRight:
            WallSearch.Position.X = WallSearch.Position.X - 1;
            break;
         case eLeft:
            WallSearch.Position.X = WallSearch.Position.X + 1;
            break;
         case eDown:
            WallSearch.Position.Y = WallSearch.Position.Y - 1;
            break;
         case eUp:
            WallSearch.Position.Y = WallSearch.Position.Y + 1;
            break;
         default:
            break;
      }
      return WallSearch;
   }

   inline WallDatas GetWallSideLeft (const WallDatas& aWallRef) {
      WallDatas WallSearch = aWallRef;
      switch (mDirection) {
         case eRight:
            WallSearch.Position.Y = WallSearch.Position.Y - 2;
            break;
         case eLeft:
            WallSearch.Position.Y = WallSearch.Position.Y + 2;
            break;
         case eDown:
            WallSearch.Position.X = WallSearch.Position.X + 2;
            break;
         case eUp:
            WallSearch.Position.X = WallSearch.Position.X - 2;
            break;
         default:
            break;
      }
      return WallSearch;
   }

   inline WallDatas GetWallSideRigth (const WallDatas& aWallRef) {
      WallDatas WallSearch = aWallRef;
      switch (mDirection) {
         case eRight:
            WallSearch.Position.Y = WallSearch.Position.Y + 2;
            break;
         case eLeft:
            WallSearch.Position.Y = WallSearch.Position.Y - 2;
            break;
         case eDown:
            WallSearch.Position.X = WallSearch.Position.X - 2;
            break;
         case eUp:
            WallSearch.Position.X = WallSearch.Position.X + 2;
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
      switch (mDirection) {
         case eRight:
         case eLeft:
            bIsBuilding = IsConstructibleVertical (aWallsDatas, aWall.Position);
            break;
         case eDown:
         case eUp:
            bIsBuilding = IsConstructibleHorizontal (aWallsDatas, aWall.Position);
            break;
         default:
            break;
      }
      return bIsBuilding;
   }

   inline bool WallPatternLeft2IsBuilding (const vector<WallDatas>& aWallsDatas, const WallDatas& aWallRef, WallDatas& aWallBuilding) {
      switch (mDirection) {
         case eRight:
            aWallBuilding.Position = Coord {aWallRef.Position.X - 1, aWallRef.Position.Y};
            aWallBuilding.Orientation = "H";
            break;
         case eLeft:
            aWallBuilding.Position = Coord {aWallRef.Position.X - 1, aWallRef.Position.Y + 2};
            aWallBuilding.Orientation = "H";
            break;
         case eDown:
            aWallBuilding.Position = Coord {aWallRef.Position.X + 2, aWallRef.Position.Y - 1};
            aWallBuilding.Orientation = "V";
            break;
         case eUp:
            aWallBuilding.Position = Coord {aWallRef.Position.X, aWallRef.Position.Y - 1};
            aWallBuilding.Orientation = "V";
            break;
         default:
            break;
      }
      return WallIsBuilding (aWallsDatas, aWallBuilding);
   }

   inline bool WallPatternRigth2IsBuilding (const vector<WallDatas>& aWallsDatas, const WallDatas& aWallRef, WallDatas& aWallBuilding) {
      switch (mDirection) {
         case eRight:
            aWallBuilding.Position = Coord {aWallRef.Position.X - 1, aWallRef.Position.Y + 2};
            aWallBuilding.Orientation = "H";
            break;
         case eLeft:
            aWallBuilding.Position = Coord {aWallRef.Position.X - 1, aWallRef.Position.Y};
            aWallBuilding.Orientation = "H";
            break;
         case eDown:
            aWallBuilding.Position = Coord {aWallRef.Position.X, aWallRef.Position.Y - 1};
            aWallBuilding.Orientation = "V";
            break;
         case eUp:
            aWallBuilding.Position = Coord {aWallRef.Position.X + 2, aWallRef.Position.Y - 1};
            aWallBuilding.Orientation = "V";
            break;
         default:
            break;
      }
      return WallIsBuilding (aWallsDatas, aWallBuilding);
   }

   inline bool WallPatternLeftIsBuilding (const vector<WallDatas>& aWallsDatas, const WallDatas& aWallRef, WallDatas& aWallBuilding) {
      bool bContinue = false;
      switch (mDirection) {
         case eRight:
            if (1 < aWallRef.Position.Y)
            {
               bContinue = true;
               aWallBuilding.Position = Coord {aWallRef.Position.X - 1, aWallRef.Position.Y - 1};
               aWallBuilding.Orientation = "H";
            }
            break;
         case eLeft:
            if (aWallRef.Position.Y < (_Height - 3))
            {
               bContinue = true;
               aWallBuilding.Position = Coord {aWallRef.Position.X - 1, aWallRef.Position.Y + 3};
               aWallBuilding.Orientation = "H";
            }
            break;
         case eDown:
            if (aWallRef.Position.X < (_Width - 3))
            {
               bContinue = true;
               aWallBuilding.Position = Coord {aWallRef.Position.X + 3, aWallRef.Position.Y - 1};
               aWallBuilding.Orientation = "V";
            }
            break;
         case eUp:
            if (1 < aWallRef.Position.X)
            {
               bContinue = true;
               aWallBuilding.Position = Coord {aWallRef.Position.X - 1, aWallRef.Position.Y - 1};
               aWallBuilding.Orientation = "V";
            }
            break;
         default:
            break;
      }
      if (bContinue)
      {
         bContinue = WallIsBuilding (aWallsDatas, aWallBuilding);
      }
      return bContinue;
   }

   inline bool WallPatternRigthIsBuilding (const vector<WallDatas>& aWallsDatas, const WallDatas& aWallRef, WallDatas& aWallBuilding) {
      bool bContinue = false;
      switch (mDirection) {
         case eRight:
            if (aWallRef.Position.Y < (_Height - 3))
            {
               bContinue = true;
               aWallBuilding.Position = Coord {aWallRef.Position.X - 1, aWallRef.Position.Y + 3};
               aWallBuilding.Orientation = "H";
            }
            break;
         case eLeft:
            if (1 < aWallRef.Position.Y)
            {
               bContinue = true;
               aWallBuilding.Position = Coord {aWallRef.Position.X - 1, aWallRef.Position.Y - 1};
               aWallBuilding.Orientation = "H";
            }
            break;
         case eDown:
            if (1 < aWallRef.Position.X)
            {
               bContinue = true;
               aWallBuilding.Position = Coord {aWallRef.Position.X - 1, aWallRef.Position.Y - 1};
               aWallBuilding.Orientation = "V";
            }
            break;
         case eUp:
            if (aWallRef.Position.X < (_Width - 3))
            {
               bContinue = true;
               aWallBuilding.Position = Coord {aWallRef.Position.X + 3, aWallRef.Position.Y - 1};
               aWallBuilding.Orientation = "V";
            }
            break;
         default:
            break;
      }
      if (bContinue)
      {
         bContinue = WallIsBuilding (aWallsDatas, aWallBuilding);
      }
      return bContinue;
   }

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
    bool    CalculPlusCourtCheminPlayer   (const CPlayerDatas& aPlayersDatas, vector<int>& aOutPlusCourtChemin);
    void    CalculCheminMinimaux          (void);
    
    string  GetNextDirection            (const vector<int>& aPlusCourtChemin);
    string  BuildWallInFrontOfPlayer    (CPlayerDatas::List& aPlayersDatas, CPlayerDatas& aPlayer, const vector<WallDatas>& aWallsBuilt);

private:
    bool CalculPlusCourtChemin  (const int aNumCaseDepart, const int aNumCaseArrivee, vector<int>& aOutPlusCourtChemin);
    
    bool BuildVerticalWall    (CPlayerDatas::List& aPlayersDatas, CPlayerDatas& aPlayer, const vector<WallDatas>& aWallsBuilt, const Coord& aCoordWall1, const Coord& aCoordWall2, WallDatas& aWallDatas, unsigned int& aLengthPCC, double& aScore);
    bool BuildHorizontalWall  (CPlayerDatas::List& aPlayersDatas, CPlayerDatas& aPlayer, const vector<WallDatas>& aWallsBuilt, const Coord& aCoordWall1, const Coord& aCoordWall2, WallDatas& aWallDatas, unsigned int& aLengthPCC, double& aScore);

    bool ChercheNewPCCPlayer (CPlayerDatas::List& aPlayersDatas, CPlayerDatas& aPlayer, const WallDatas& aWallDatas, unsigned int& aLengthPCC, double& aScore);
    void SelectionneBestWall (const CPlayerDatas& aPlayerDatas,
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
                              double& aScore);
    
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

bool CIA::CalculPlusCourtCheminPlayer (const CPlayerDatas& aPlayersDatas, vector<int>& aOutPlusCourtChemin)
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
      bReturn = CalculPlusCourtChemin (aPlayersDatas.GetPosition ().X + aPlayersDatas.GetPosition ().Y * mWidth, CasesArrivees[i], PCC);
      if (bReturn && (1 < PCC.size()) && (PCC.size() < DistanceMin)) {
         DistanceMin = PCC.size();
         aOutPlusCourtChemin = PCC;
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
    while ((NbIter != NbIterMax) && (NumCaseCourante != aNumCaseArrivee) && (NumCaseCourante < POIDS_MUR)) {
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

string CIA::BuildWallInFrontOfPlayer (CPlayerDatas::List& aPlayersDatas, CPlayerDatas& aMe, const vector<WallDatas>& aWallsBuilt)
{
   string WallBuilding;

   CPlayerDatas::List::iterator ItPlayer = aPlayersDatas.begin ();
   while (((*ItPlayer) != aMe) && (WallBuilding.empty ())) {
      CPlayerDatas& Player = *ItPlayer;
      const vector<int> PCCPlayer = Player.GetPCC ();

      WallFound BestWallDatas;
      for (unsigned int iCase = 0; iCase < (PCCPlayer.size () - 1); ++iCase)
      {

         const unsigned int iCaseNext = iCase + 1;

         bool bConstructible = false;
         WallDatas      WallDatas;
         unsigned int   LengthPCC = 0;
         double         ScorePCC  = 0.0;

         // Si la prochaine case est celle de gauche
         if (PCCPlayer[iCaseNext] == (PCCPlayer[iCase] - 1))
         {
            Coord CoordNewWall1 = {PCCPlayer[iCase] % mWidth, PCCPlayer[iCase] / mWidth};
            Coord CoordNewWall2 = {CoordNewWall1.X, CoordNewWall1.Y - 1};

            bConstructible = BuildVerticalWall (aPlayersDatas, Player, aWallsBuilt, CoordNewWall1, CoordNewWall2, WallDatas, LengthPCC, ScorePCC);
         }
         // Si la prochaine case est celle de droite
         else if (PCCPlayer[iCaseNext] == (PCCPlayer[iCase] + 1))
         {
            Coord CoordNewWall1 = {PCCPlayer[iCase] % mWidth + 1, PCCPlayer[iCase] / mWidth};
            Coord CoordNewWall2 = {CoordNewWall1.X, CoordNewWall1.Y - 1};

            bConstructible = BuildVerticalWall (aPlayersDatas, Player, aWallsBuilt, CoordNewWall1, CoordNewWall2, WallDatas, LengthPCC, ScorePCC);
         }
         // Si la prochaine case est celle du haut
         else if (PCCPlayer[iCaseNext] == (PCCPlayer[iCase] - mWidth))
         {
            Coord CoordNewWall1 = {PCCPlayer[iCase] % mWidth, PCCPlayer[iCase] / mWidth};
            Coord CoordNewWall2 = {CoordNewWall1.X - 1, CoordNewWall1.Y};

            bConstructible = BuildHorizontalWall (aPlayersDatas, Player, aWallsBuilt, CoordNewWall1, CoordNewWall2, WallDatas, LengthPCC, ScorePCC);
         }
         // Si la prochaine case est celle du bas
         else if (PCCPlayer[iCaseNext] == (PCCPlayer[iCase] + mWidth))
         {
            Coord CoordNewWall1 = {PCCPlayer[iCase] % mWidth, PCCPlayer[iCase] / mWidth + 1};
            Coord CoordNewWall2 = {CoordNewWall1.X - 1, CoordNewWall1.Y};

            bConstructible = BuildHorizontalWall (aPlayersDatas, Player, aWallsBuilt, CoordNewWall1, CoordNewWall2, WallDatas, LengthPCC, ScorePCC);
         }

         if (bConstructible)
         {
            //cerr << "Mur " << WallDatas.ToString () << " L=" << LengthPCC << " S=" << ScorePCC << endl;
            //cerr << "BestWallDatas.Length " << BestWallDatas.Length << endl;
            //if (BestWallDatas.Length < LengthPCC) // TODO <=
            if (BestWallDatas.Score < ScorePCC)
            {
               //cerr << "Best" << endl;

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

bool CIA::BuildVerticalWall (CPlayerDatas::List& aPlayersDatas, CPlayerDatas& aPlayer, const vector<WallDatas>& aWallsBuilt, const Coord& aCoordWall1, const Coord& aCoordWall2, WallDatas& aWallDatas, unsigned int& aLengthPCC, double& aScore)
{
   bool bConstructible1 = IsConstructibleVertical (aWallsBuilt, aCoordWall1);
   bool bConstructible2 = IsConstructibleVertical (aWallsBuilt, aCoordWall2);

   WallDatas WallDatas1 = {aCoordWall1, "V"};
   WallDatas WallDatas2 = {aCoordWall2, "V"};
   unsigned int LengthPCC1 = 99;
   unsigned int LengthPCC2 = 99;
   double ScorePCC1 = 0;
   double ScorePCC2 = 0;

   if (bConstructible1 && bConstructible2)
   {
      // Calcul du nouveau PCC pour le joueur avec le Mur 1
      bConstructible1 = ChercheNewPCCPlayer (aPlayersDatas, aPlayer, WallDatas1, LengthPCC1, ScorePCC1);

      // Calcul du nouveau PCC pour le joueur avec le Mur 2
      bConstructible2 = ChercheNewPCCPlayer (aPlayersDatas, aPlayer, WallDatas2, LengthPCC2, ScorePCC2);

      // On sélectionne le meilleur
      SelectionneBestWall (aPlayer, bConstructible1, WallDatas1, LengthPCC1, ScorePCC1, bConstructible2, WallDatas2, LengthPCC2, ScorePCC2, aWallDatas, aLengthPCC, aScore);
   }
   else if (bConstructible1)
   {
      // Calcul du nouveau PCC pour le joueur avec le Mur 1
      bConstructible1 = ChercheNewPCCPlayer (aPlayersDatas, aPlayer, WallDatas1, LengthPCC1, ScorePCC1);

      if (bConstructible1)
      {
         aWallDatas  = WallDatas1;
         aLengthPCC  = LengthPCC1;
         aScore      = ScorePCC1;
      }
   }
   else if (bConstructible2)
   {
      // Calcul du nouveau PCC pour le joueur avec le Mur 2
      bConstructible2 = ChercheNewPCCPlayer (aPlayersDatas, aPlayer, WallDatas2, LengthPCC2, ScorePCC2);

      if (bConstructible2)
      {
         aWallDatas  = WallDatas2;
         aLengthPCC  = LengthPCC2;
         aScore      = ScorePCC2;
      }
   }
   return bConstructible1 || bConstructible2;
}

bool CIA::BuildHorizontalWall (CPlayerDatas::List& aPlayersDatas, CPlayerDatas& aPlayer, const vector<WallDatas>& aWallsBuilt, const Coord& aCoordWall1, const Coord& aCoordWall2, WallDatas& aWallDatas, unsigned int& aLengthPCC, double& aScore)
{
   bool bConstructible1 = IsConstructibleHorizontal (aWallsBuilt, aCoordWall1);
   bool bConstructible2 = IsConstructibleHorizontal (aWallsBuilt, aCoordWall2);

   WallDatas WallDatas1 = {aCoordWall1, "H"};
   WallDatas WallDatas2 = {aCoordWall2, "H"};
   unsigned int LengthPCC1 = 99;
   unsigned int LengthPCC2 = 99;
   double ScorePCC1 = 0;
   double ScorePCC2 = 0;

   if (bConstructible1 && bConstructible2)
   {
      // Calcul du nouveau PCC pour le joueur avec le Mur 1
      bConstructible1 = ChercheNewPCCPlayer (aPlayersDatas, aPlayer, WallDatas1, LengthPCC1, ScorePCC1);

      // Calcul du nouveau PCC pour le joueur avec le Mur 2
      bConstructible2 = ChercheNewPCCPlayer (aPlayersDatas, aPlayer, WallDatas2, LengthPCC2, ScorePCC2);

      // On Garde le meilleur
      SelectionneBestWall (aPlayer, bConstructible1, WallDatas1, LengthPCC1, ScorePCC1, bConstructible2, WallDatas2, LengthPCC2, ScorePCC2, aWallDatas, aLengthPCC, aScore);
   }
   else if (bConstructible1)
   {
      // Calcul du nouveau PCC pour le joueur avec le Mur 1
      bConstructible1 = ChercheNewPCCPlayer (aPlayersDatas, aPlayer, WallDatas1, LengthPCC1, ScorePCC1);

      if (bConstructible1)
      {
         aWallDatas  = WallDatas1;
         aLengthPCC  = LengthPCC1;
         aScore      = ScorePCC1;
      }
   }
   else if (bConstructible2)
   {
      // Calcul du nouveau PCC pour le joueur avec le Mur 2
      bConstructible2 = ChercheNewPCCPlayer (aPlayersDatas, aPlayer, WallDatas2, LengthPCC2, ScorePCC2);

      if (bConstructible2)
      {
         aWallDatas  = WallDatas2;
         aLengthPCC  = LengthPCC2;
         aScore      = ScorePCC2;
      }
   }
   return bConstructible1 || bConstructible2;
}

bool CIA::ChercheNewPCCPlayer (CPlayerDatas::List& aPlayersDatas, CPlayerDatas& aPlayer, const WallDatas& aWallDatas, unsigned int& aLengthPCC, double& aScore)
{
   bool bConstructible = true;
   AjoutMurMatriceGrapheLite (aWallDatas, false);
   CalculCheminMinimaux ();
   CPlayerDatas::List::iterator itPlayerDatas = aPlayersDatas.begin ();
   while (itPlayerDatas != aPlayersDatas.end () && bConstructible) {
      vector<int> PCC;
      bConstructible = CalculPlusCourtCheminPlayer (*itPlayerDatas, PCC);

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

//   vector <int>::const_iterator IterPlusCourtChemin = aPlayersDatas[aIdPlayer].GetPCC ().begin ();
//   for (; IterPlusCourtChemin != aPlayersDatas[aIdPlayer].GetPCC ().end (); ++IterPlusCourtChemin)
//   {
//      cerr << (*IterPlusCourtChemin) << " ";
//   }
//   cerr << endl;
//   cerr << "Mur " << aWallDatas.ToString () << " L=" << aLengthPCC << endl;

   return bConstructible;
}

void CIA::SelectionneBestWall (const CPlayerDatas& aPlayerDatas,
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
                  aScore         = aScore1;
               }
               else
               {
                  aBestWallDatas = aWallDatas2;
                  aLengthBestPCC = aLengthPCC2;
                  aScore         = aScore2;
               }
            }
            else
            {
               if (0 == (aWallDatas1.Position.Y % 2))
               {
                  aBestWallDatas = aWallDatas2;
                  aLengthBestPCC = aLengthPCC2;
                  aScore         = aScore2;
               }
               else
               {
                  aBestWallDatas = aWallDatas1;
                  aLengthBestPCC = aLengthPCC1;
                  aScore         = aScore1;
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
                  aScore         = aScore1;
               }
               else
               {
                  aBestWallDatas = aWallDatas2;
                  aLengthBestPCC = aLengthPCC2;
                  aScore         = aScore2;
               }
            }
            else
            {
               if (0 == (aWallDatas1.Position.X % 2))
               {
                  aBestWallDatas = aWallDatas2;
                  aLengthBestPCC = aLengthPCC2;
                  aScore         = aScore2;
               }
               else
               {
                  aBestWallDatas = aWallDatas1;
                  aLengthBestPCC = aLengthPCC1;
                  aScore         = aScore1;
               }
            }
         }
      }
      else
      {
         aBestWallDatas = aWallDatas1;
         aLengthBestPCC = aLengthPCC1;
         aScore         = aScore1;
         //if (aLengthPCC1 < aLengthPCC2) // TODO
         if (aScore1 < aScore2)
         {
            aBestWallDatas = aWallDatas2;
            aLengthBestPCC = aLengthPCC2;
            aScore         = aScore2;
         }
      }
   }
   else if (abConstructible1)
   {
      aBestWallDatas = aWallDatas1;
      aLengthBestPCC = aLengthPCC1;
      aScore         = aScore1;
   }
   else if (abConstructible2)
   {
      aBestWallDatas = aWallDatas2;
      aLengthBestPCC = aLengthPCC2;
      aScore         = aScore2;
   }
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
   int playerCount; // number of players (2 or 3)
   cin >> _Width >> _Height >> playerCount >> _myId; cin.ignore();

   CPlayerDatas::List     PlayersDatasOrdonnes;
   vector<WallDatas>      WallsDatas;
   bool                   bBuildingOn = false;

   bool           bModePatternOn = true;
   bool           bPatternStarted = false;
   unsigned int   CurrentStep = 0;
   WallDatas      WallDatasRef;

   Measure measure;

   CIA IA (_Width, _Height);

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

      measure.start ();

      IA.CalculCheminMinimaux ();

      bool  bAvance = true;
      bool  bPattern = false;
      string Action;

      CPlayerDatas::List::iterator ItPlayerOrdonnes;
      for (ItPlayerOrdonnes = PlayersDatasOrdonnes.begin ();
           ItPlayerOrdonnes != PlayersDatasOrdonnes.end ();
           ++ItPlayerOrdonnes) {
         vector<int> PCC;
         IA.CalculPlusCourtCheminPlayer (*ItPlayerOrdonnes, PCC);
         (*ItPlayerOrdonnes).SetPCC (PCC);
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
         if (bModePatternOn && ((*Me).GetPCC ().size () > 2)) {
            CPlayerDatas::List::iterator OtherPlayer = PlayersDatasOrdonnes.begin ();
            while ((OtherPlayer != PlayersDatasOrdonnes.end ()) && (OtherPlayer != Me)) {
               ++OtherPlayer;
            }
            if (!bPatternStarted && (*Me).IsPositionStartPattern ()) {
               if ((*OtherPlayer).GetPCC ().size () > 2) {
                  if ((*Me).StartPattern (WallsDatas, WallDatasRef)) {
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
               if ((*OtherPlayer).GetPCC ().size () > 5) {
                  CurrentStep = (*Me).GetNextStep (WallsDatas, CurrentStep, WallDatasRef, Action);
                  if (CurrentStep == 0) {
                     bPattern = false;
                     bPatternStarted = false;
                  }
                  else
                  {
                     bPattern = true;
                  }
               }
            }
         }
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

      if (bPattern) {
         cerr << "P";
         if (bPatternStarted) {
            cerr << " - Start";
         }
         cerr << endl;
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

      cout << Action << " " << measure.get() << endl; // action: LEFT, RIGHT, UP, DOWN or "putX putY putOrientation" to place a wall
   }
}
