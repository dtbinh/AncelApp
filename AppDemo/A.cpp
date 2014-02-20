#include "A.h"


AStart::AStart(char *mapInfo, int width, int height, int start, int end)
{
	Width  = width;
	Height = height;
	Start  = start;
	End    = end;

	//把二维数组保存到一维数组中去，便于信息的处理
	map = new char[Width * Height];
	for (int i = 0; i < Width * Height; i++)
	{
		//map[i] = mapInfo[i / width][i % width];
		map[i] = mapInfo[i];
	}

	//记录每一个节点的位置信息
	rect = new Rect[Width * Height];
	for (int i = 0; i < (Width * Height); i++)
	{
		rect[i].x = i % Width;
		rect[i].y = i / Width;
	}

	//初始化起点
	rect[Start].g_value = 0;
	rect[Start].h_value = get_h_value(Start);
	rect[Start].pre = NULL;

	//把起点加入open_list中
	open_list.push_back(Start);
}

AStart::~AStart()
{
	if (map != NULL)
	{
		delete[] map;
	}
	if (rect != NULL)
	{
		delete[] rect;
	}
}

int AStart::get_g_value(int pos)
{
	//只允许玩家往上下左右四个方向行走，所以这里的g值只需要在父节点的g值上加10
	return (rect[pos].pre->g_value + 10);
}

int AStart::get_h_value(int pos)
{
	//返回该点到终点的Manhattan距离，乘以10是为了方便计算机计算
	return (10 * (abs(End / Width - pos / Width) + abs(End % Width - pos % Width)));
}

void AStart::getResultPath()
{
	Rect *temp = &rect[End];
	while (temp != NULL)
	{
		result.push_back(*temp);
		temp = temp->pre;
	}
	return;
}

bool AStart::isReachable(int pos)
{
	if ((pos / Width < Height) && (pos / Width >= 0) &&
		(pos % Width < Width)  && (pos % Width >= 0))
	{
		return true;
	}
	else
	{
		return false;
	}
}

//如果pos不可达或者它在close_list中则跳过它，否则，进行如下操作
//如果pos不在open_list中则加入open_list，并把当前方格设置为它的父亲
//如果pos在open_list中则检查g的大小，如果更小则把它的父亲设置为当前方格
bool AStart::testRoad(int pos, int cur)
{
	if (isReachable(pos))
	{
		if (pos == End)
		{
			rect[pos].pre = &rect[cur];
			return true;
		}
		if (map[pos] != '#') //1代表障碍物，0则可通行
		{
			if (close_list.end() == find(close_list.begin(), close_list.end(), pos))
			{
				std::list<int>::iterator iter = find(open_list.begin(), open_list.end(), pos);
				if (iter == open_list.end())
				{
					open_list.push_back(pos);
					rect[pos].pre = &rect[cur];
					rect[pos].h_value = get_h_value(pos);
					rect[pos].g_value = get_g_value(pos);
				}
				else
				{
					if ((rect[cur].g_value + 10) < rect[pos].g_value)
					{
						rect[pos].pre = &rect[cur];
						rect[pos].g_value = get_g_value(pos);
					}
				}
			}
		}
	}
	return false;
}

bool AStart::Find()
{
	//遍历open_list，查找F值最小的节点作为当前要处理的节点
	//如果open_list为空，则表明没有解决方案
	if (open_list.empty())
	{
		return false;
	}

	int f_value = 0;
	int min_f_value = -1;
	std::list<int>::iterator iter, save;
	for (iter = open_list.begin(); iter != open_list.end(); iter++)
	{
		f_value = rect[*iter].g_value + rect[*iter].h_value;
		//这里的min==f也会重新给它赋值，导致open_list中靠后的元素具有更高的优先级
		//不过无关紧要
		if ((min_f_value == -1) || (min_f_value >= f_value))
		{
			min_f_value = f_value;
			save = iter;
		}
	}

	//把这个F值最小的节点移到close_list中
	int cur = *save;
	close_list.push_back(cur);
	open_list.erase(save);


	//对当前方格的上下左右相邻方格进行测试
	//如果终点进入了open_list则结束
	int up    = cur - Width;
	int down  = cur + Width;
	int left  = cur - 1;
	int right = cur + 1;
	if (true == testRoad(up, cur))
	{
		return true;
	}
	if (true == testRoad(down, cur))
	{
		return true;
	}
	if (true == testRoad(left, cur))
	{
		return true;
	}
	if (true == testRoad(right, cur))
	{
		return true;
	}

	return Find();
}