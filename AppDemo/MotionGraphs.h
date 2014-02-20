#ifndef __MotionGraphs_h_
#define __MotionGraphs_h_

#include <vector>
#include <OgreSingleton.h>
#include <PanelView/BasePanelViewItem.h>
#include <MotionSyn.h>
#include <BaseLayout/BaseLayout.h>
#include <OgreVector3.h>
#include "Motion.h"
#include "Skeleton.h"
#include "Animation.h"
#include "Path.h"
#include "MousePicker.h"
#include "SceneEntityManager.h"
#include <math.h>
#include <list>
#include <algorithm>
#include <stack>

#define MAX 100010
#define MAXNODE 10005
#define LENGTH_TO_CUT 10.0//7.5

#define M 100
#define N 100


namespace AncelApp
{
	const int inf = 0x3f3f3f3f;

	typedef struct EDGE
	{
		int v, next, c;
		EDGE(int vv = 0, int cc = 0): v(vv), c(cc){}
		bool operator < (const EDGE& r) const {return c > r.c;}
	}edge;

	struct Point
	{
		int x, y, time;
		int  x_next, y_next;
/*
		bool operator < (const Point& a) const
		{
			return time > a.time;
		}*/
	};

	struct PPoint
	{
		double x, y;
	};

/*

	struct APoint{   
		int x, y;   
		APoint(){}   
		APoint(int __x, int __y){ x = __x, y = __y; }  
		int getDistanceFrom(const APoint& __p){ return abs(x - __p.x) + abs(y - __p.y); }  
	};  
	struct ClosedAPoint: APoint{   
		int previous;   
		ClosedAPoint(APoint& __p, int __pre): APoint(__p), previous(__pre) {}   
	};  
	struct OpenAPoint: ClosedAPoint{   
		int distanceToSource, distanceToTarget, assessValue;   
		OpenAPoint(APoint &__p, int __per, int __ds, int __dt) : ClosedAPoint(__p, __per) ,   
			distanceToSource(__ds), distanceToTarget(__dt), assessValue(__dt + __ds) {}  
	};  */

	class MotionGraphs : public Ogre::Singleton<MotionGraphs>, public wraps::BaseLayout
	{
	public:
		MotionGraphs();
		~MotionGraphs();

		//void WriteMotionToDat();
		
		void addEdge(const int a, const int b);
		void Tarjan(int v);
		void solve(int n);
		friend class MotionGraphsPanel;
	//	friend class SceneEntityManager;
		void randomPlay(int src);
		void randomWalk(int src);
		void randomRun(int src);
		void randomJump(int src);
		bool nodeIn(int a);
		void dijkstra(int n, int src);
		void getMap();
		void setMap(int col, int row);


		bool findPath();
		void findIt();
//		void printMap();

		int bfs();
		void doIt();
		bool overMap(int x, int y);
		bool isParallel(Point p1, Point p2, Point p3);
		//Path* createPlayPath();
		//void calculateHandlePoint(Eigen::MatrixXd &position, int traceNum);

	protected:
	private:
		EDGE e[MAX];
		int edgeNum;
		int head[MAX];
		int dfsnum[MAX],dfsNum,low[MAX];
		int sccnum[MAX],sccNum;
		int instack[MAX],st[MAX],top;

		int maxNum;
		int maxNode[MAXNODE];
		int scccount[MAXNODE];

		int tempcount;
		int tempNode[MAXNODE];		// for find the path;

		int pathPlay[MAXNODE];   // for play
		int pathPlayCount;    
		int countPlay[MAXNODE];
		int countHelp;

		int dist[MAX];
		int prevv[MAX];
		int vis[MAX];

//--A-Star
//		APoint startAPoint, endAPoint;
//		stack<APoint> aPath;


//--BFS
		Point startPoint, endPoint, final;
		Point record[M][N];
		char map[M][N];
		bool mark[M][N];
		stack<Point> S;
		Point Ppath[MAX];
		int PpCount;
		
		PPoint tempPpath[MAX];
		int tempPpCount;

		Point tempPp[MAX];
		int tempPpCc;
		
	};
}

#endif