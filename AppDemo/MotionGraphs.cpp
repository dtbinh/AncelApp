#include "MotionGraphs.h"
#include "MotionManager.h"
#include "AppDemo.h"
#include "AppUtility.h"
#include "CommandManager.h"
#include "StatePanel.h"
#include <MyGUI.h>
#include "AnimationManager.h"
#include <math.h>
#include <vector>
#include <algorithm>
#include <stack>
#include "A.h"
#include <list>
#include <time.h>

using namespace AncelApp;

//#define FOR_DEBUG
#define BFS

template<> MotionGraphs* Ogre::Singleton<MotionGraphs>::msSingleton = nullptr;

MotionGraphs::MotionGraphs()
{
	edgeNum = 0;
	pathPlayCount = 0;
	dfsNum = 0;
	top = 0;
	sccNum = 0;
	maxNum = -1;
	tempcount = 0;
	countHelp = 0;
	tempPpCount = 0;
	tempPpCc = 0;
	memset(head,-1,sizeof(head));
	memset(dfsnum,0,sizeof(dfsnum));
	memset(instack,0,sizeof(instack));
	memset(scccount,0,sizeof(scccount));
	memset(tempNode,0,sizeof(tempNode));
	memset(pathPlay,0,sizeof(pathPlay));
	memset(countPlay,0,sizeof(countPlay));
	memset(prevv, -1, sizeof(prevv));
	memset(Ppath,0,sizeof(Ppath));
	memset(tempPpath,0,sizeof(tempPpath));
	memset(tempPp,0,sizeof(tempPp));
	PpCount = 0;
	memset(vis,0,sizeof(vis));
	for (int i = 0; i < MAX; i++)
		dist[i] = inf;
	for (int i = 0; i < M; i++)
	{
		for(int j = 0; j < N; j++)
			map[i][j] = '.';
	}
}

MotionGraphs::~MotionGraphs()
{

}

void MotionGraphs::addEdge(const int a, const int b)
{
	e[edgeNum].v = b;
	e[edgeNum].c = 1;
	e[edgeNum].next = head[a];
	head[a] = edgeNum++;
}

void MotionGraphs::Tarjan(int i)
{
	dfsnum[i] = low[i] = ++dfsNum;
	st[top++] = i;
	instack[i] = 1;
	int j = head[i];
	for(j = head[i];j != -1;j = e[j].next)
	{
		int v = e[j].v;
		if(dfsnum[v] == 0)
		{
			Tarjan(v);
			if(low[i] > low[v])
				low[i] = low[v];
		}
		else if(instack[v])
		{
			if(low[i] > dfsnum[v])
				low[i] = dfsnum[v];
		}
	}
	if(dfsnum[i] == low[i])
	{
#ifdef FOR_DEBUG
		printf("sccNum: %d\n",sccNum);
#endif		
		do
		{
			top--;
#ifdef FOR_DEBUG
			printf("+ %d \n", st[top]);
#endif
			sccnum[st[top]] = sccNum;
			scccount[sccNum]++;
			instack[st[top]] = 0;
		}while(top >= 0 && st[top] != i);
		sccNum++;
	}
}

void MotionGraphs::solve(int n)
{
	int i;
	for (i = 0; i < n; i++)
	{
		if (dfsnum[i] == 0)
		{
			Tarjan(i);
		}	
	}
	int tt = -1;
	for (i = 0; i < sccNum; i++)
	{
		if (scccount[i] > maxNum)
		{
			maxNum = scccount[i];
			tt = i;
		}
	}
	if (tt == -1)
	{
		printf("error in tarjan!");
	}
	for (i = 0; i < n; i++)
	{
		if(sccnum[i] == tt)
		{
			tempNode[tempcount++] = i;
		}
	}
}

void MotionGraphs::randomPlay(int src)
{
	if (pathPlayCount >= 600)
	{
		return;
	}
	pathPlay[pathPlayCount++] = src;
	countPlay[src]++;
	countHelp++;
	int j = head[src];
	//int mm = 99999;
	//int index = -1;
	for (;j != -1; j = e[j].next)
	{
		int v = e[j].v;
		//if (mm > countPlay[v] && nodeIn(v))
		//{
		//	mm = countPlay[v];
		//	index = v;
		//}

		if (nodeIn(v))// && countPlay[v]*tempcount < countHelp+1)
		{
			randomPlay(v);
		}
	}
	//randomPlay(index);
}
void MotionGraphs::randomWalk(int src)
{
	if (pathPlayCount >= 600)
	{
		return;
	}
	pathPlay[pathPlayCount++] = src;
	countPlay[src]++;
	countHelp++;
	int j = head[src];
	for (;j != -1; j = e[j].next)
	{
		int v = e[j].v;
		if (nodeIn(v))
		{
			randomPlay(v);
		}
	}
}

void MotionGraphs::randomRun(int src)
{
	if (pathPlayCount >= 600)
	{
		return;
	}
	pathPlay[pathPlayCount++] = src;
	countPlay[src]++;
	countHelp++;
	int j = head[src];
	for (;j != -1; j = e[j].next)
	{
		int v = e[j].v;
		if (nodeIn(v))
		{
			randomPlay(v);
		}
	}
}

void MotionGraphs::randomJump(int src)
{
	if (pathPlayCount >= 600)
	{
		return;
	}
	pathPlay[pathPlayCount++] = src;
	countPlay[src]++;
	countHelp++;
	int j = head[src];
	for (;j != -1; j = e[j].next)
	{
		int v = e[j].v;
		if (nodeIn(v))
		{
			randomPlay(v);
		}
	}
}

bool MotionGraphs::nodeIn(int a)
{
	for (int i = 0; i < tempcount; i++)
	{
		if (tempNode[i] == a)
		{
			return true;
		}
	}
	return false;
}

void MotionGraphs::dijkstra(int n,int src)
{
	EDGE mv;
	int i, j, k, pre;
	priority_queue<EDGE> que;
	vis[src] = 1; dist[src] = 0;
	que.push(EDGE(src, 0));
	for (pre = src, i = 1; i < n; i++)
	{
		for (j = head[pre]; j != -1; j = e[j].next)
		{
			k = e[j].v;
			if (vis[k] == 0 && dist[pre] + e[j].c < dist[k])// && nodeIn(k) )
			{
				dist[k] = dist[pre] + e[j].c;
				que.push(EDGE(e[j].v, dist[k]));
				prevv[k] = pre;
			}
		}
		while(!que.empty() && vis[que.top().v] == 1)
			que.pop();
		if(que.empty()) break;
		mv = que.top();
		que.pop();
		vis[pre = mv.v] = 1;
	}
}

void MotionGraphs::getMap()
{
//	map[startPoint.y][startPoint.x] = 'x';
//	map[endPoint.y][endPoint.x] = '@';
	for(int i = 0; i < M; i++)
	{
		for (int j = 0; j < N; j++)
		{
			printf("%c",map[i][j]);
		}
		printf("3\n");
	}

}

void MotionGraphs::setMap(int col, int row)
{
	map[col][row] = '#';
}
//---------A----------
/*
bool MotionGraphs::findPath()
{
	vector<OpenAPoint> openList;
	vector<ClosedAPoint> closeList;

	openList.push_back(OpenAPoint(startAPoint, -1, 0, startAPoint.getDistanceFrom(endAPoint)));

	int direction[4][2] = {{0, -1}, {0, 1}, {1, 0}, {-1, 0}};
	while(!openList.empty())
	{
		vector<OpenAPoint>::iterator current = openList.begin();
		
		for (vector<OpenAPoint>::iterator it = openList.begin(); it != openList.end(); it++)
		{
			if(it->assessValue < current->assessValue)
				current = it;
		}

		if (current->x == endAPoint.x && current->y == endAPoint.y)
		{
			aPath.push(endAPoint);
			int previous = current->previous;
			while(previous != -1)
			{
				aPath.push( *(closeList.begin() + previous));
				previous = (closeList.begin() + previous)->previous;
			}
			return true;
		}

		OpenAPoint currentPoint = *current;
		openList.erase(current);
		closeList.push_back(currentPoint);	
		map[currentPoint.x][currentPoint.y] = '-';

		for (int i = 0; i < 4; i++)
		{
			int nextX = currentPoint.x + direction[i][0];
			int nextY = currentPoint.y + direction[i][1];

			APoint newPoint(nextX, nextY);

			if (nextX < 0 || nextY < 0 || nextX >= M || nextY >= N || map[nextX][nextY] == '#')
				continue;

			for (vector<ClosedAPoint>::iterator it = closeList.begin(); it != closeList.end(); it++)
			{
				if(it->x == nextX && it->y == nextY)
					goto SearchNext;
			}

			for (vector<OpenAPoint>::iterator it = openList.begin(); it != openList.end(); it++)
			{
				if(it->x == nextX && it->y == nextY)
					goto SearchNext;
			}

			openList.push_back(OpenAPoint(newPoint, closeList.size()-1,currentPoint.distanceToSource+1,endAPoint.getDistanceFrom(newPoint)));

SearchNext:;
		}
	}
	return false;
}
*/

void MotionGraphs::findIt()
{
	list<Rect>::iterator it;
	AStart *astart;
	
	char *map11;
	map11 = new char[M*N];
	int k = 0;
	for (int i = 0; i < M; i++)
	{
		for(int j = 0; j < N; j++)
		{
			map11[k++] = map[i][j];
		}
	}
	astart = new AStart(map11, M, N, 49*M+53, 59*M+29);

	bool find = astart->Find();
	if (find)
	{
		astart->getResultPath();
		it = astart->result.begin();
		while(it != astart->result.end())
		{
			cout << it->y << ":" << it->x << endl;	
			it++;
		}
	}
	else
	{
		cout << "No path" << endl;
	}
}
//--------------------------------------------------------------

int MotionGraphs::bfs()
{
	memset(mark, 0, sizeof(mark));
	int direction[4][2] = {{0, -1}, {0, 1}, {1, 0}, {-1, 0}};
	queue<Point> Q;
	Point first, next;
	first.x = startPoint.x;
	first.y = startPoint.y;
	first.time = 0;	
	Q.push(first);
	while(!Q.empty())
	{
		first = Q.front();
		Q.pop();
		if (first.x == endPoint.x && first.y == endPoint.y)
		{
			final.x = first.x;
			final.y = first.y;
			final.x_next = 0;
			final.y_next = 0;
			return final.time;
		}
		for (int i = 0; i < 4; i++)
		{
			next = first;
			next.x = first.x + direction[i][0];
			next.y = first.y + direction[i][1];
			if (overMap(next.x,next.y) || map[next.x][next.y] == '#' || mark[next.x][next.y])
				continue;
			mark[next.x][next.y] = true;
			if(map[next.x][next.x] == '.')
			{
				next.time++;
			}
			Q.push(next);
			first.x_next = next.x;
			first.y_next = next.y;
			record[next.x][next.y] = first;
		}
	}
	return -1;
}

void MotionGraphs::doIt()
{
	startPoint.x = 54;
	startPoint.y = 50;

//	endPoint.x = 30;
//	endPoint.y = 60;
//	clock_t startTime, finishTime;
//	startTime = clock();
#ifdef BFS
	//clock_t startTime, finishTime;
	//startTime = clock();
	int ret = bfs();
	//finishTime = clock();
	//cout << (double)(finishTime - startTime) / CLOCKS_PER_SEC << "seconds" << endl;
	if (ret == -1)
	{
		cout << "no path!" << endl;
	}
	else
	{
		S.push(final);
		Point temp = record[final.x][final.y];
		while(!(temp.x == startPoint.x && temp.y == startPoint.y))
		{
			S.push(temp);
			temp = record[temp.x][temp.y];
		}
		S.push(temp);
		while(!S.empty())
		{
			temp =	S.top();
			S.pop();
			if (map[temp.x][temp.y] == '.' && !(temp.x_next == startPoint.x && temp.y_next == startPoint.y))
			{
				//cout << "(" <<temp.x << "," << temp.y << ")";
				map[temp.x][temp.y] = '+';
				Ppath[PpCount].x = temp.x;
				Ppath[PpCount++].y = temp.y;
			}
		}
	}
#else
	list<Rect>::iterator it;
	AStart *astart;

	char *map11;
	map11 = new char[M*N];
	int k = 0;
	for (int i = 0; i < M; i++)
	{
		for(int j = 0; j < N; j++)
		{
			map11[k++] = map[i][j];
		}
	}
	astart = new AStart(map11, M, N, (startPoint.y-1)*M+startPoint.x-1, (endPoint.y-1)*M+endPoint.x-1);
	//clock_t startTime, finishTime;
	//startTime = clock();
	bool find = astart->Find();
	//finishTime = clock();
	//cout << (double)(finishTime - startTime) / CLOCKS_PER_SEC << "seconds" << endl;
	if (find)
	{
	//	S.push(endPoint);
		Point temp;
		astart->getResultPath();
		it = astart->result.begin();

		while(it != astart->result.end())
		{
			//cout << it->y << ":" << it->x << endl;	
			temp.x = it->x + 1;
			temp.y = it->y + 1;
			S.push(temp);
			it++;
		}
	//	S.push(startPoint);
		while(!S.empty())
		{
			temp = S.top();
			S.pop();
			if (map[temp.x][temp.y] == '.')
			{
				map[temp.x][temp.y] = '+';
				Ppath[PpCount].x = temp.x;
				Ppath[PpCount++].y = temp.y;
			}
		}
	}
	else
	{
		cout << "No path" << endl;
	}

#endif

//	finishTime = clock();
//	cout << (double)(finishTime - startTime) / CLOCKS_PER_SEC << "seconds" << endl;
//轻量移位

	tempPp[tempPpCc++] = Ppath[0];
	int turn = 0;
	if (Ppath[0].x == Ppath[1].x)
		turn = 1;
	else if (Ppath[0].y == Ppath[1].y)
		turn = -1;
	for (int i = 1; i < PpCount - 1; i++)
	{
		if ((Ppath[i].x == Ppath[i+1].x) && turn == 1)
		{
			turn = 1;
		}			
		else if ((Ppath[i].x == Ppath[i+1].x) && turn == -1)
		{
			tempPp[tempPpCc++] = Ppath[i];
			turn = 1;
		}
		else if ((Ppath[i].y == Ppath[i+1].y) && turn == 1)
		{
			tempPp[tempPpCc++] = Ppath[i];
			turn = -1;
		}
		else if ((Ppath[i].y == Ppath[i+1].y) && turn == -1)
		{
			turn = -1;
		}
	}
	tempPp[tempPpCc++] = Ppath[PpCount-1];
	for (int i = 0; i < PpCount; i++)
	{
		int xx = Ppath[i].x;
		int yy = Ppath[i].y;
		if (map[xx][yy+1] == '#')
		{
			for (int j = 0; j < tempPpCc-1; j++)
			{
				if (tempPp[j].y == yy && tempPp[j+1].y == yy)
				{
					tempPp[j].y -= 1;
					tempPp[j+1].y -= 1;
				}
			}
		}
		if (map[xx][yy-1] == '#')
		{
			for (int j = 0; j < tempPpCc-1; j++)
			{
				if (tempPp[j].y == yy && tempPp[j+1].y == yy)
				{
					tempPp[j].y += 1;
					tempPp[j+1].y += 1;
				}
			}
		}
		if (map[xx-1][yy] == '#')
		{
			for (int j = 0; j < tempPpCc-1; j++)
			{
				if (tempPp[j].x == xx && tempPp[j+1].x == xx)
				{
					tempPp[j].x += 1;
					tempPp[j+1].x += 1;
				}
			}
		}
		if (map[xx+1][yy] == '#')
		{
			for (int j = 0; j < tempPpCc-1; j++)
			{
				if (tempPp[j].x == xx && tempPp[j+1].x == xx)
				{
					tempPp[j].x -= 1;
					tempPp[j+1].x -= 1;
				}
			}
		}
	}

	for (int i = 0; i < tempPpCc; i++)
	{
		tempPp[i].x = tempPp[i].x * 8 - 400;
		tempPp[i].y = tempPp[i].y * 8 - 400;
	}


	double disTotal = 0.0;
	double playDis[100];

	for (int i = 0; i < tempPpCc-1; i++)
	{
		playDis[i] = sqrt((double)(tempPp[i+1].x - tempPp[i].x)*(tempPp[i+1].x - tempPp[i].x) + (double)(tempPp[i+1].y - tempPp[i].y)*(tempPp[i+1].y - tempPp[i].y));
		disTotal += playDis[i];
	}

	int frameForPlay = (int)(disTotal/0.25);
	int kk = 0;
	int first = 1;
	for (int i = 0; i < tempPpCc - 2; i++)
	{
		if (i  == 0)
		{
			if (playDis[i] > LENGTH_TO_CUT)
			{
				kk = (int)(frameForPlay * (playDis[i] - LENGTH_TO_CUT) /  disTotal);
			//	kk = 3;
				for (double add = 0.0; add <= 1.0; add += (double)(1.0/(double)kk))
				{
					if (tempPp[i].x == tempPp[i+1].x)
					{
						tempPpath[tempPpCount].x = tempPp[i].x;
						if(tempPp[i+1].y > tempPp[i].y)
							tempPpath[tempPpCount++].y = add * (tempPp[i+1].y - LENGTH_TO_CUT) + (1.0 - add) * tempPp[i].y;
						else
							tempPpath[tempPpCount++].y = add * (tempPp[i+1].y + LENGTH_TO_CUT) + (1.0 - add) * tempPp[i].y;
					}
					else
					{
						if(tempPp[i+1].x > tempPp[i].x)
							tempPpath[tempPpCount].x = add * (tempPp[i+1].x - LENGTH_TO_CUT) + (1.0 - add) * tempPp[i].x;
						else
							tempPpath[tempPpCount].x = add * (tempPp[i+1].x + LENGTH_TO_CUT) + (1.0 - add) * tempPp[i].x;
						tempPpath[tempPpCount++].y = tempPp[i].y;
					}
				}
				first = 0;
			}
		}
		else
		{
			if(playDis[i] > 2 * LENGTH_TO_CUT)
			{
				kk = (int)(frameForPlay * (playDis[i] - 2*LENGTH_TO_CUT) /  disTotal);
				//kk = 3;
				for (double add = 0.0; add <= 1.0; add += (double)(1.0/(double)kk))
				{
					if (tempPp[i].x == tempPp[i+1].x)
					{
						tempPpath[tempPpCount].x = tempPp[i].x;
						if(tempPp[i+1].y > tempPp[i].y)
							tempPpath[tempPpCount++].y = add * (tempPp[i+1].y - LENGTH_TO_CUT) + (1.0 - add) * (tempPp[i].y + LENGTH_TO_CUT);
						else
							tempPpath[tempPpCount++].y = add * (tempPp[i+1].y + LENGTH_TO_CUT) + (1.0 - add) * (tempPp[i].y - LENGTH_TO_CUT);
					}
					else
					{
						if(tempPp[i+1].x > tempPp[i].x)
							tempPpath[tempPpCount].x = add * (tempPp[i+1].x - LENGTH_TO_CUT) + (1.0 - add) * (tempPp[i].x + LENGTH_TO_CUT);
						else
							tempPpath[tempPpCount].x = add * (tempPp[i+1].x + LENGTH_TO_CUT) + (1.0 - add) * (tempPp[i].x - LENGTH_TO_CUT);
						tempPpath[tempPpCount++].y = tempPp[i].y;
					}
				}
			}
		}

		//插入八种情况
		Point p1 = tempPp[i], p2 = tempPp[i+1], p3 = tempPp[i+2];
		kk = (int)((LENGTH_TO_CUT*M_PI/2.0)/0.25);
		//kk = 3;
		if(p1.y < p2.y && p1.x == p2.x && p2.x > p3.x && p2.y == p3.y )
		{
			for(int ij = 1; ij < kk; ij++)
			{
				tempPpath[tempPpCount].x = p2.x-LENGTH_TO_CUT + LENGTH_TO_CUT*cos(ij*M_PI/(2.0*kk));
				tempPpath[tempPpCount++].y = p2.y-LENGTH_TO_CUT + LENGTH_TO_CUT*sin(ij*M_PI/(2.0*kk));
			}
		}
		else if(p1.y == p2.y && p1.x > p2.x && p2.x == p3.x && p2.y < p3.y )
		{
			for(int ij = 1; ij < kk; ij++)
			{
				tempPpath[tempPpCount].x = p2.x+LENGTH_TO_CUT - LENGTH_TO_CUT*sin(ij*M_PI/(2.0*kk));
				tempPpath[tempPpCount++].y = p2.y+LENGTH_TO_CUT - LENGTH_TO_CUT*cos(ij*M_PI/(2.0*kk));
			}
		}
		else if(p1.y == p2.y && p1.x < p2.x && p2.x == p3.x && p2.y < p3.y )
		{
			for(int ij = 1; ij < kk; ij++)
			{
				tempPpath[tempPpCount].x = p2.x-LENGTH_TO_CUT + LENGTH_TO_CUT*sin(ij*M_PI/(2.0*kk));
				tempPpath[tempPpCount++].y = p2.y+LENGTH_TO_CUT - LENGTH_TO_CUT*cos(ij*M_PI/(2.0*kk));
			}
		}
		else if(p1.y == p2.y && p1.x > p2.x && p2.x == p3.x && p2.y > p3.y )
		{
			for(int ij = 1; ij < kk; ij++)
			{
				tempPpath[tempPpCount].x = p2.x+LENGTH_TO_CUT - LENGTH_TO_CUT*sin(ij*M_PI/(2.0*kk));
				tempPpath[tempPpCount++].y = p2.y-LENGTH_TO_CUT + LENGTH_TO_CUT*cos(ij*M_PI/(2.0*kk));
			}
		}
		else if(p1.y == p2.y && p1.x < p2.x && p2.x == p3.x && p2.y > p3.y )
		{
			for(int ij = 1; ij < kk; ij++)
			{
				tempPpath[tempPpCount].x = p2.x-LENGTH_TO_CUT + LENGTH_TO_CUT*sin(ij*M_PI/(2.0*kk));
				tempPpath[tempPpCount++].y = p2.y-LENGTH_TO_CUT + LENGTH_TO_CUT*cos(ij*M_PI/(2.0*kk));
			}
		}	
		else if(p1.y > p2.y && p1.x == p2.x && p2.x < p3.x && p2.y == p3.y )
		{
			for(int ij = 1; ij < kk; ij++)
			{
				tempPpath[tempPpCount].x = p2.x+LENGTH_TO_CUT - LENGTH_TO_CUT*cos(ij*M_PI/(2.0*kk));
				tempPpath[tempPpCount++].y = p2.y+LENGTH_TO_CUT - LENGTH_TO_CUT*sin(ij*M_PI/(2.0*kk));
			}
		}
		else if(p1.y > p2.y && p1.x == p2.x && p2.x > p3.x && p2.y == p3.y )
		{
			for(int ij = 1; ij < kk; ij++)
			{
				tempPpath[tempPpCount].x = p2.x-LENGTH_TO_CUT + LENGTH_TO_CUT*cos(ij*M_PI/(2.0*kk));
				tempPpath[tempPpCount++].y = p2.y+LENGTH_TO_CUT - LENGTH_TO_CUT*sin(ij*M_PI/(2.0*kk));
			}
		}		
		else if(p1.y < p2.y && p1.x == p2.x && p2.x < p3.x && p2.y == p3.y )
		{
			for(int ij = 1; ij < kk; ij++)
			{
				tempPpath[tempPpCount].x = p2.x+LENGTH_TO_CUT - LENGTH_TO_CUT*cos(ij*M_PI/(2.0*kk));
				tempPpath[tempPpCount++].y = p2.y-LENGTH_TO_CUT + LENGTH_TO_CUT*sin(ij*M_PI/(2.0*kk));
			}
		}
	}
	//最后一个情况
	kk = (int)(frameForPlay * (playDis[tempPpCc-2] - LENGTH_TO_CUT) /  disTotal);
	//kk = 3;
	for (double add = 0.0; add <= 1.0; add += (double)(1.0/(double)kk))
	{
		if (tempPp[tempPpCc-2].x == tempPp[tempPpCc-1].x)
		{
			tempPpath[tempPpCount].x = tempPp[tempPpCc-1].x;
			if(tempPp[tempPpCc-1].y > tempPp[tempPpCc-2].y)
				tempPpath[tempPpCount++].y = add * tempPp[tempPpCc-1].y + (1.0 - add) * (tempPp[tempPpCc-2].y + LENGTH_TO_CUT);
			else
				tempPpath[tempPpCount++].y = add * tempPp[tempPpCc-1].y + (1.0 - add) * (tempPp[tempPpCc-2].y - LENGTH_TO_CUT);
		}
		else
		{
			if(tempPp[tempPpCc-1].x > tempPp[tempPpCc-2].x)
				tempPpath[tempPpCount].x = add * tempPp[tempPpCc-1].x + (1.0 - add) * (tempPp[tempPpCc-2].x + LENGTH_TO_CUT);
			else
				tempPpath[tempPpCount].x = add * tempPp[tempPpCc-1].x + (1.0 - add) * (tempPp[tempPpCc-2].x - LENGTH_TO_CUT);
			tempPpath[tempPpCount++].y = tempPp[tempPpCc-1].y;
		}
	}

}

bool MotionGraphs::overMap(int x, int y)
{
	if (x < 0 || x >= M || y < 0 || y >= N)
	{
		return true;
	}
	else
		return false;
}

bool MotionGraphs::isParallel(Point p1, Point p2, Point p3)
{
	if ((p1.x == p2.x && p1.x == p3.x) || (p1.y == p2.y && p1.y == p3.y)) // 平行
	{
		return true;
	}
	else
		return false;
}