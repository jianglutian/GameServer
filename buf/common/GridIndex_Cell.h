
#ifndef GRIDINDEX_CELL_H_
#define GRIDINDEX_CELL_H_

#include "core/UnorderedContainer.h"
#include "core/Enumerator.h"
#include "core/RTime.h"
#include "core/Logger.h"
#include "Pos.h"

namespace buf
{

/**
 * N屏对象管理系统
 *
 *  (0,0)
 *    --------------------------------> x
 *    |    
 *    |   [x][x][x]
 *    |   [x][1][2][3]
 *    |   [x][4][*][6][x]
 *    |      [7][8][9][x]
 *    |         [x][x][x]
 *    |
 *    |
 *    |    [x][1][2][3][x]
 *    |    [x][4][5][6][x]
 *    |    [x][7][8][9][x]
 *    |
 *    |
 *    V                                (w,h)
 *    y
 *
 **/

#define GRID_INVALID ((u32_t)-1)

extern u32_t GRID_WIDTH; // XXX: >= 3
extern u32_t GRID_HEIGHT; // XXX: >= 3
extern u32_t GRID_RADIUS; // XXX: >= 1

#if 0
#define PRINTVEC(x) printvec(x)
#define PRINT(...) printf(__VA_ARGS__)
#else
#define PRINTVEC(x)
#define PRINT(...)
#endif

static void setGridWH(const Pos& gridWH)
{
    (void)setGridWH;
    if (gridWH.x < 3 || gridWH.y < 3)
        return;
    GRID_WIDTH  = gridWH.x;
    GRID_HEIGHT = gridWH.y;
}

static void setGridRadius(u32_t radius)
{
    (void)setGridRadius;
    if (!radius)
        return;
    GRID_RADIUS = radius;
}

static bool posToGrid(const Pos& mapWH, const Pos& p, PosI& pi)
{
    (void)posToGrid;
    if (p.x < mapWH.x && p.y < mapWH.y)
    {
        pi = ((p.y/GRID_HEIGHT) * ((mapWH.x+GRID_WIDTH-1)/GRID_WIDTH)) + (p.x/GRID_WIDTH);
        return true;
    }
    return false;
}

// XXX: Just for comparing
static u32_t getDistance(const Pos& p1, const Pos& p2)
{
    (void)getDistance;
    return abs(p1.x - p2.x) + abs(p1.y - p2.y);
}

// XXX:
// T must be drived from MapObject
//      need T::ObjectType_Max,
//
template <typename T>
class GridIndex
{
public:
    typedef UNORDERED_SET<T*> ObjectSet;
    typedef typename ObjectSet::iterator OSIterator;
    typedef UNORDERED_MAP<PosI, ObjectSet> ObjectContainer;
    typedef typename ObjectContainer::iterator OCIterator;

    typedef UNORDERED_MAP<PosI, PosIVec_t> GridContainer;
    typedef typename GridContainer::iterator GridIterator;
    typedef typename GridContainer::const_iterator ConstGridIterator;

public:

    GridIndex() {}
    virtual ~GridIndex() {}

    // XXX:
    //  mapWH - the max cell(x,y) of screen
    void init(const Pos& mapWH);

    inline Pos getMapWH() const { return _mapWH; }
    inline u32_t getWidth() const { return _mapWH.x; }
    inline u32_t getHeight() const { return _mapWH.y; }

    // 取得第p屏相关联的屏
    const PosIVec_t& getAssociateGrid(const PosI& p) const;
    // 取得dir方向切入p屏时差分屏
    const PosIVec_t& getEnterGridByDir(const PosI& p, int dir) const;
    // 取得dir方向离开p屏时差分屏
    const PosIVec_t& getLeaveGridByDir(const PosI& p, int dir) const;

    // 取得range范围内的屏幕
    // range - 以屏为单位
    void getGridByRange(const PosI& p, int range, PosIVec_t& piv) const;
    void getGridByRange(const Pos& p, int range, PosIVec_t& piv) const;

    bool refresh(T* obj, const Pos& p);
    void remove(T* obj);

    // p2在p1的哪个方向
    int getGridDirect(const PosI& p1, const PosI& p2) const;
    // 两个屏是否存在关联
    bool isGridAssociated(const PosI& g1, const PosI& g2) const;
    // 两个点是否在同屏内
    bool isInOneGrid(const Pos& p1, const Pos& p2) const;

    bool posValidate(const Pos& p) const { return p.x < _gridWH.x && p.y < _gridWH.y; }
    bool posValidate(const PosI& pi) const { return pi < _idxMax; }

    void enumerate(Enumerator<T>& e, int type = T::ObjectType_Max);
    void enumerate(Enumerator<T>& e, const PosI& g, int type = T::ObjectType_Max);

protected:
    // 整个地图的格子宽和高
    Pos _mapWH;
    // 横向屏幕(Grid)个数及纵向屏幕个数(每个屏里由多个格子组成)
    Pos _gridWH;
    // 最大屏幕索引
    PosI _idxMax;
    // 每个屏的相关屏
    GridContainer _gridsIdx;
    // 进入时差分
    GridContainer _enterGrids[POS_UNKONW];
    // 离开时差分
    GridContainer _leaveGrids[POS_UNKONW];
    // 各屏幕里的对象容器
    ObjectContainer _objsIdx[T::ObjectType_Max];
    // 所有对象容器
    ObjectSet _objs[T::ObjectType_Max];
};

inline void side4(int d, PosVec_t& up, PosVec_t& right, PosVec_t& down, PosVec_t& left)
{
    for (int i = 0; i <= d; ++i) // UP DOWN
    {
        int x = i;
        int y = d;

        up.push_back(Pos(x,-y));
        down.push_back(Pos(x,y));

        if (x)
        {
            int x = -i;
            up.push_back(Pos(x,-y));
            down.push_back(Pos(x,y));
        }
    }

    for (int i = 0; i <= d; ++i) // RIGHT LEFT
    {
        int x = d;
        int y = i;

        right.push_back(Pos(x,y));
        left.push_back(Pos(-x,y));

        if (y)
        {
            int y = -i;
            right.push_back(Pos(x,y));
            left.push_back(Pos(-x,y));
        }
    }
}

static int _signed[] = {1, -1};

template <typename T>
void GridIndex<T>::init(const Pos& mapWH)
{
    _mapWH = mapWH;
    _gridWH.x = (mapWH.x + GRID_WIDTH - 1) / GRID_WIDTH;
    _gridWH.y = (mapWH.y + GRID_HEIGHT - 1) / GRID_HEIGHT;
    _idxMax = _gridWH.x * _gridWH.y;

    PosIVec_t piv;
    PosIVec_t yapiv;

    int d = GRID_RADIUS;
    if (d < 1)
        return;

    PosVec_t pv[4]; // UP RIGHT DOWN LEFT
    side4(d, pv[0], pv[1], pv[2], pv[3]);

    PRINTVEC(pv[0]);
    PRINTVEC(pv[1]);
    PRINTVEC(pv[2]);
    PRINTVEC(pv[3]);

    char buf[128] = {0};
    SNPRINTF(buf, sizeof(buf), "Initialize map grids size (%d, %d): ", _gridWH.x, _gridWH.y);
    TimeFilm tf(buf, &sLog);

    for (u32_t i = 0; i < _idxMax; ++i)
    {
        int gridX = i % _gridWH.x;
        int gridY = i / _gridWH.x;

        PRINT("(%d, %d)[%d]: ", gridX, gridY, gridY * _gridWH.x + gridX);

        for (int x = 0; x <= d; ++x)
        {
            for (int y = 0; y <= d; ++y)
            {
                if (!(!x && !y))
                {
                    for (int c = 0; c < 2; ++c)
                    {
                        int X = gridX + x * _signed[c];
                        int Y = gridY + y * _signed[c];

                        if (X >= 0 && Y >= 0 && X < (int)_gridWH.x && Y < (int)_gridWH.y)
                            piv.push_back(Y * _gridWH.x + X);
                    }

                    // X对称
                    if (x && y)
                    {
                        int X = gridX + x;
                        int Y = gridY - y;

                        if (X >= 0 && Y >= 0 && X < (int)_gridWH.x && Y < (int)_gridWH.y)
                            piv.push_back(Y * _gridWH.x + X);
                    }

                    // Y对称
                    if (x && y)
                    {
                        int X = gridX - x;
                        int Y = gridY + y;

                        if (X >= 0 && Y >= 0 && X < (int)_gridWH.x && Y < (int)_gridWH.y)
                            piv.push_back(Y * _gridWH.x + X);
                    }
                }
                else
                {
                    if (gridX >= 0 && gridY >= 0 && gridX < (int)_gridWH.x && gridY < (int)_gridWH.y)
                        piv.push_back(gridY * _gridWH.x + gridX);
                }
            }
        }

        PRINTVEC(piv);

        _gridsIdx[i] = piv;
        piv.clear();

        int g = 2*d+1;

        // 切入
        for (int dir = 0; dir < 8; ++dir)
        {
            int r = dir>>1;
            for (int c = 0; c < g; ++c)
            {
                Pos& p = pv[r][c];
                int X = gridX + p.x;
                int Y = gridY + p.y;

                if (X >= 0 && Y >= 0 && X < (int)_gridWH.x && Y < (int)_gridWH.y)
                    piv.push_back(Y * _gridWH.x + X);
            }

            if (dir % 2)
            {
                int r = ((dir+1)&0x7)>>1;
                for (int c = 0; c < g; ++c)
                {
                    Pos& p = pv[r][c];
                    int X = gridX + p.x;
                    int Y = gridY + p.y;

                    if ((dir == POS_DOWNRIGHT || dir == POS_UPLEFT) && p.x == p.y)
                        continue;
                    if ((dir == POS_UPRIGHT || dir == POS_DOWNLEFT) && p.x == -p.y)
                        continue;
                    if (X >= 0 && Y >= 0 && X < (int)_gridWH.x && Y < (int)_gridWH.y)
                        piv.push_back(Y * _gridWH.x + X);
                }
            }

            PRINT("切入: ");
            PRINTVEC(piv);

            _enterGrids[dir][i] = piv;

            yapiv.clear();
            piv.clear();
        }

        // 退出
        for (int dir = 0; dir < 8; ++dir)
        {
            int r = ((dir>>1)+2)&0x3;
            for (int c = 0; c < g; ++c)
            {
                Pos& p = pv[r][c];
                int X = gridX + p.x;
                int Y = gridY + p.y;

                if (X >= 0 && Y >= 0 && X < (int)_gridWH.x && Y < (int)_gridWH.y)
                    piv.push_back(Y * _gridWH.x + X);
            }

            if (dir % 2)
            {
                int r = ((((dir+1)&0x7)>>1)+2)&0x3;
                for (int c = 0; c < g; ++c)
                {
                    Pos& p = pv[r][c];
                    int X = gridX + p.x;
                    int Y = gridY + p.y;

                    if ((dir == POS_DOWNRIGHT || dir == POS_UPLEFT) && p.x == p.y)
                        continue;
                    if ((dir == POS_UPRIGHT || dir == POS_DOWNLEFT) && p.x == -p.y)
                        continue;
                    if (X >= 0 && Y >= 0 && X < (int)_gridWH.x && Y < (int)_gridWH.y)
                        piv.push_back(Y * _gridWH.x + X);
                }
            }

            PRINT("退出: ");
            PRINTVEC(piv);

            _leaveGrids[dir][i] = piv;

            yapiv.clear();
            piv.clear();
        }
    }

}

template <typename T>
const PosIVec_t& GridIndex<T>::getAssociateGrid(const PosI& p) const
{
    static PosIVec_t null;
    if (p < _idxMax)
    {
        ConstGridIterator i = _gridsIdx.find(p);
        if (i != _gridsIdx.end())
            return i->second;
    }
    return null;
}

template <typename T>
const PosIVec_t& GridIndex<T>::getEnterGridByDir(const PosI& p, int dir) const
{
    static PosIVec_t null;
    if (p < _idxMax && dir < POS_UNKONW)
    {
        ConstGridIterator i = _enterGrids[dir].find(p);
        if (i != _enterGrids[dir].end())
            return i->second;
    }
    return null;
}

template <typename T>
const PosIVec_t& GridIndex<T>::getLeaveGridByDir(const PosI& p, int dir) const
{
    static PosIVec_t null;
    if (p < _idxMax && dir < POS_UNKONW)
    {
        ConstGridIterator i = _leaveGrids[dir].find(p);
        if (i != _leaveGrids[dir].end())
            return i->second;
    }
    return null;
}

template <typename T>
void GridIndex<T>::getGridByRange(const PosI& p, int range, PosIVec_t& piv) const
{
    if (range < 1)
        return;

    int gridX = p % _gridWH.x;
    int gridY = p / _gridWH.x;

    int d = range;
    for (int x = 0; x <= d; ++x)
    {
        for (int y = 0; y <= d; ++y)
        {
            if (!(!x && !y))
            {
                for (int c = 0; c < 2; ++c)
                {
                    int X = gridX + x * _signed[c];
                    int Y = gridY + y * _signed[c];

                    if (X >= 0 && Y >= 0 && X < (int)_gridWH.x && Y < (int)_gridWH.y)
                        piv.push_back(Y * _gridWH.x + X);
                }

                // X对称
                if (x && y)
                {
                    int X = gridX + x;
                    int Y = gridY - y;

                    if (X >= 0 && Y >= 0 && X < (int)_gridWH.x && Y < (int)_gridWH.y)
                        piv.push_back(Y * _gridWH.x + X);
                }

                // Y对称
                if (x && y)
                {
                    int X = gridX - x;
                    int Y = gridY + y;

                    if (X >= 0 && Y >= 0 && X < (int)_gridWH.x && Y < (int)_gridWH.y)
                        piv.push_back(Y * _gridWH.x + X);
                }
            }
            else
            {
                if (gridX >= 0 && gridY >= 0 && gridX < (int)_gridWH.x && gridY < (int)_gridWH.y)
                    piv.push_back(gridY * _gridWH.x + gridX);
            }
        }
    }
}

template <typename T>
void GridIndex<T>::getGridByRange(const Pos& p, int range, PosIVec_t& piv) const
{
    PosI pi;
    if (!posToGrid(getMapWH(), p, pi))
        return;
    getGridByRange(pi, range, piv);
}

template <typename T>
bool GridIndex<T>::refresh(T* obj, const Pos& p)
{
    if (!obj)
        return false;
    int type = obj->getType();
    if (type >= T::ObjectType_Max)
        return false;

    PosI pi;
    if (!posToGrid(getMapWH(), p, pi))
        return false;

    if (obj->isInWorld())
    {
        PosI old = obj->getGrid();
        if (obj->setPos(getMapWH(), p))
        {
            if (pi != old)
            {
                _objsIdx[type][old].erase(obj);
                return _objsIdx[type][pi].insert(obj).second;
            }
        }
    }
    else
    {
        if (obj->setPos(getMapWH(), p))
        {
            if (_objsIdx[type][pi].insert(obj).second)
            {
                if (_objs[type].insert(obj).second)
                {
                    obj->setInWorld(true);
                    return true;
                }
                else
                    _objsIdx[type][pi].erase(obj);
            }
        }
    }
    return false;
}

template <typename T>
void GridIndex<T>::remove(T* obj)
{
    if (!obj)
        return false;
    int type = obj->getType();
    if (type >= T::ObjectType_Max)
        return false;
    OSIterator i = _objs[type].find(obj);
    if (i == _objs[type].end())
        return false;

    _objsIdx[type][obj->getGrid()].erase(obj);
    _objs[type].erase(obj);
    obj->setInWorld(false);

    return true;
}

template <typename T>
int GridIndex<T>::getGridDirect(const PosI& p1, const PosI& p2) const
{
    unsigned int x1 = p1 % _gridWH.x;
    unsigned int y1 = p1 / _gridWH.x;
    unsigned int x2 = p2 % _gridWH.x;
    unsigned int y2 = p2 / _gridWH.x;

    if (x1 == x2 && y1 >= y2+1)
    {
        return POS_UP;
    }
    else if (x1+1 <= x2 && y1 >= y2+1)
    {
        return POS_UPRIGHT;
    }
    else if (x1+1 <= x2 && y1 == y2)
    {
        return POS_RIGHT;
    }
    else if (x1+1 <= x2 && y1+1 <= y2)
    {
        return POS_DOWNRIGHT;
    }
    else if (x1 == x2 && y1+1 <= y2)
    {
        return POS_DOWN;
    }
    else if (x1 >= 1+x2 && y1+1 <= y2)
    {
        return POS_DOWNLEFT;
    }
    else if (x1 >= 1+x2 && y1 == y2)
    {
        return POS_LEFT;
    }
    else if (x1 >= 1+x2 && y1 >= 1+y2)
    {
        return POS_UPLEFT;
    }
    else
    {
        // TODO: log
        return POS_UP;
    }
}

template <typename T>
bool GridIndex<T>::isGridAssociated(const PosI& g1, const PosI& g2) const
{
    int x1, y1 , x2 ,y2;
    x1 = g1 % _gridWH.x;
    y1 = g1 / _gridWH.x;
    x2 = g2 % _gridWH.x;
    y2 = g2 / _gridWH.x;
    if(abs(x1 - x2) <= GRID_RADIUS && abs(y1 - y2) <= GRID_RADIUS)
        return true;
    return false;
}

template <typename T>
bool GridIndex<T>::isInOneGrid(const Pos& p1, const Pos& p2) const
{
    PosI pi1, pi2;
    if (posToGrid(_gridWH, p1, pi1) && (posToGrid(_gridWH, p2, pi2)) && pi1 == pi2)
        return true;
    return false;
}

template <typename T>
void GridIndex<T>::enumerate(Enumerator<T>& e, int type)
{
    int start = 0;
    int end = 0;

    if (type < T::ObjectType_Max)
    {
        start = type;
        end = type+1;
    }
    else
    {
        end = T::ObjectType_Max;
    }

    for (int i = start; i < end; ++i)
    {
        for (OSIterator it = _objs[i].begin(); it != _objs[i].end(); ++it)
        {
            int t = (*it)->getType();
            if (type != T::ObjectType_Max && t != type)
                continue;
            if (!e(*it))
                return;
        }
    }
}

template <typename T>
void GridIndex<T>::enumerate(Enumerator<T>& e, const PosI& g, int type)
{
    if (g > _idxMax)
        return;

    int start = 0;
    int end = 0;

    if (type < T::ObjectType_Max)
    {
        start = type;
        end = type+1;
    }
    else
    {
        end = T::ObjectType_Max;
    }

    for (int i = start; i < end; ++i)
    {
        for (OSIterator it = _objsIdx[i][g].begin(); it != _objsIdx[i][g].end(); ++it)
        {
            int t = (*it)->getType();
            if (type != T::ObjectType_Max && t != type)
                continue;
            if (!e(*it))
                return;
        }
    }
}

} // namespace buf

#endif // GRIDINDEX_CELL_H_

/* vim: set ai si nu sm smd hls is ts=4 sm=4 bs=indent,eol,start */

