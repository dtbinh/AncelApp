#include "A.h"


AStart::AStart(char *mapInfo, int width, int height, int start, int end)
{
	Width  = width;
	Height = height;
	Start  = start;
	End    = end;

	//�Ѷ�ά���鱣�浽һά������ȥ��������Ϣ�Ĵ���
	map = new char[Width * Height];
	for (int i = 0; i < Width * Height; i++)
	{
		//map[i] = mapInfo[i / width][i % width];
		map[i] = mapInfo[i];
	}

	//��¼ÿһ���ڵ��λ����Ϣ
	rect = new Rect[Width * Height];
	for (int i = 0; i < (Width * Height); i++)
	{
		rect[i].x = i % Width;
		rect[i].y = i / Width;
	}

	//��ʼ�����
	rect[Start].g_value = 0;
	rect[Start].h_value = get_h_value(Start);
	rect[Start].pre = NULL;

	//��������open_list��
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
	//ֻ������������������ĸ��������ߣ����������gֵֻ��Ҫ�ڸ��ڵ��gֵ�ϼ�10
	return (rect[pos].pre->g_value + 10);
}

int AStart::get_h_value(int pos)
{
	//���ظõ㵽�յ��Manhattan���룬����10��Ϊ�˷�����������
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

//���pos���ɴ��������close_list���������������򣬽������²���
//���pos����open_list�������open_list�����ѵ�ǰ��������Ϊ���ĸ���
//���pos��open_list������g�Ĵ�С�������С������ĸ�������Ϊ��ǰ����
bool AStart::testRoad(int pos, int cur)
{
	if (isReachable(pos))
	{
		if (pos == End)
		{
			rect[pos].pre = &rect[cur];
			return true;
		}
		if (map[pos] != '#') //1�����ϰ��0���ͨ��
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
	//����open_list������Fֵ��С�Ľڵ���Ϊ��ǰҪ����Ľڵ�
	//���open_listΪ�գ������û�н������
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
		//�����min==fҲ�����¸�����ֵ������open_list�п����Ԫ�ؾ��и��ߵ����ȼ�
		//�����޹ؽ�Ҫ
		if ((min_f_value == -1) || (min_f_value >= f_value))
		{
			min_f_value = f_value;
			save = iter;
		}
	}

	//�����Fֵ��С�Ľڵ��Ƶ�close_list��
	int cur = *save;
	close_list.push_back(cur);
	open_list.erase(save);


	//�Ե�ǰ����������������ڷ�����в���
	//����յ������open_list�����
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