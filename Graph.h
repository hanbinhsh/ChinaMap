#ifndef GRAPH_H
#define GRAPH_H
#include "qdebug.h"
#include <QString>
#include "queue_Sq.h"

#define CUSTOM_VERTEX_NUM 30                    //可自己创建的节点数
#define ORIGINAL_VERTEX_NUM 34                  //原地图节点数
#define MAX_VERTEX_NUM ORIGINAL_VERTEX_NUM+CUSTOM_VERTEX_NUM     //最大节点数 = 原地图节点数+可自己创建的节点数
typedef struct ArcCell{
    int w;                           //权值
    //此处还可以存储其他信息
}ArcCell,AdjMatrix[MAX_VERTEX_NUM][MAX_VERTEX_NUM];

typedef struct MGraph{
    AdjMatrix arcs;                  //邻接矩阵
}MGraph;

#define TRUE 1
#define FALSE 0
#define OK 1
#define ERROR 0
#define INF 0x3f3f3f3f
// TODO 换成for读取或从文件读取地图
int MapRelation[MAX_VERTEX_NUM][MAX_VERTEX_NUM];
int MapRelation_O[MAX_VERTEX_NUM][MAX_VERTEX_NUM]=
    { {0,INF,INF,INF,INF,INF,INF,INF,INF,408,INF,INF,INF,INF,INF,INF,INF,INF,629,262,INF,114,INF,INF,INF,INF,INF,INF,INF,INF,INF,INF,INF,INF},//1
     {INF,0,INF,INF,INF,INF,INF,INF,INF,INF,INF,625,1248,INF,INF,INF,INF,INF,INF,INF,INF,INF,INF,INF,614,INF,INF,INF,INF,INF,261,INF,INF,INF},
      {INF,INF,0,691,INF,INF,INF,471,INF,INF,INF,INF,INF,INF,452,INF,INF,INF,INF,INF,INF,INF,INF,INF,INF,INF,INF,INF,INF,INF,INF,255,INF,INF},
      {INF,INF,691,0,774,INF,459,INF,INF,INF,INF,INF,INF,INF,INF,INF,502,INF,INF,INF,INF,INF,INF,INF,INF,INF,INF,INF,565,INF,INF,INF,133,108},
      {INF,INF,INF,774,0,INF,INF,INF,INF,INF,INF,429,INF,INF,INF,INF,461,INF,INF,INF,INF,INF,INF,INF,INF,INF,INF,INF,648,INF,324,INF,INF,INF},
      {INF,INF,INF,INF,INF,0,INF,INF,INF,INF,INF,INF,INF,INF,INF,INF,INF,INF,INF,INF,INF,INF,INF,INF,INF,INF,INF,240,INF,INF,INF,INF,INF,INF},
      {INF,INF,INF,459,INF,INF,0,INF,INF,INF,INF,INF,INF,INF,INF,INF,INF,INF,INF,INF,INF,INF,INF,INF,INF,INF,INF,INF,INF,INF,INF,INF,INF,INF},
      {INF,INF,471,INF,INF,INF,INF,0,INF,INF,INF,INF,INF,INF,454,245,INF,162,INF,INF,INF,INF,INF,INF,INF,INF,INF,INF,INF,INF,INF,INF,INF,INF},
      {INF,INF,INF,INF,INF,INF,INF,INF,0,INF,INF,INF,INF,INF,INF,268,INF,INF,INF,INF,INF,INF,INF,312,INF,INF,INF,INF,INF,467,INF,INF,INF,INF},
      {408,INF,INF,INF,INF,INF,INF,INF,INF,0,INF,INF,INF,INF,INF,INF,INF,INF,INF,INF,337,INF,INF,INF,INF,INF,INF,INF,INF,INF,INF,INF,INF,INF},//10
      {INF,INF,INF,INF,INF,INF,INF,INF,INF,INF,0,INF,INF,INF,INF,541,INF,INF,INF,267,INF,270,INF,INF,INF,INF,INF,INF,INF,370,INF,INF,INF,INF},
      {INF,625,INF,INF,429,INF,INF,INF,INF,INF,INF,0,INF,INF,INF,INF,INF,INF,INF,INF,INF,INF,INF,INF,INF,INF,INF,INF,INF,INF,INF,INF,INF,INF},
     {INF,1248,INF,INF,INF,INF,INF,INF,INF,INF,INF,INF,0,INF,INF,INF,INF,INF,INF,INF,INF,INF,INF,INF,INF,INF,INF,INF,INF,INF,INF,INF,INF,INF},
     {INF,INF,INF,INF,INF,INF,INF,INF,INF,INF,INF,INF,INF,0,INF,INF,INF,INF,INF,INF,INF,INF,1440,INF,503,196,343,INF,INF,INF,766,INF,INF,INF},
      {INF,INF,452,INF,INF,INF,INF,454,INF,INF,INF,INF,INF,INF,0,465,INF,INF,INF,INF,INF,INF,INF,253,INF,INF,INF,INF,289,INF,INF,INF,INF,INF},
      {INF,INF,INF,INF,INF,INF,INF,245,268,INF,541,INF,INF,INF,465,0,INF,268,INF,INF,INF,INF,INF,INF,INF,INF,INF,INF,INF,565,INF,INF,INF,INF},
      {INF,INF,INF,502,461,INF,INF,INF,INF,INF,INF,INF,INF,INF,INF,INF,0,INF,INF,INF,INF,INF,INF,INF,INF,INF,INF,INF,INF,INF,INF,INF,INF,INF},
      {INF,INF,INF,INF,INF,INF,INF,162,INF,INF,INF,INF,INF,INF,INF,268,INF,0,INF,INF,INF,INF,INF,INF,INF,INF,INF,INF,INF,INF,INF,INF,INF,INF},
      {629,INF,INF,INF,INF,INF,INF,INF,INF,INF,INF,INF,INF,INF,INF,INF,INF,INF,0,INF,INF,607,INF,INF,INF,INF,INF,271,INF,INF,INF,INF,INF,INF},
      {262,INF,INF,INF,INF,INF,INF,INF,INF,INF,267,INF,INF,INF,INF,INF,INF,INF,INF,0,173,INF,INF,INF,INF,INF,INF,INF,INF,374,INF,INF,INF,INF},//20
      {INF,INF,INF,INF,INF,INF,INF,INF,INF,337,INF,INF,INF,INF,INF,INF,INF,INF,INF,173,0,INF,INF,INF,511,INF,557,INF,INF,358,INF,INF,INF,INF},
      {114,INF,INF,INF,INF,INF,INF,INF,INF,INF,270,INF,INF,INF,INF,INF,INF,INF,607,INF,INF,0,INF,INF,INF,INF,INF,INF,INF,INF,INF,INF,INF,INF},
     {INF,INF,INF,INF,INF,INF,INF,INF,INF,INF,INF,INF,INF,1440,INF,INF,INF,INF,INF,INF,INF,INF,0,INF,INF,INF,INF,INF,INF,INF,INF,INF,INF,INF},
      {INF,INF,INF,INF,INF,INF,INF,INF,312,INF,INF,INF,INF,INF,253,INF,INF,INF,INF,INF,INF,INF,INF,0,INF,INF,INF,INF,294,467,757,INF,INF,INF},
      {INF,614,INF,INF,INF,INF,INF,INF,INF,INF,INF,INF,INF,503,INF,INF,INF,INF,INF,INF,511,INF,INF,INF,0,INF,INF,INF,INF,430,INF,INF,INF,INF},
      {INF,INF,INF,INF,INF,INF,INF,INF,INF,INF,INF,INF,INF,196,INF,INF,INF,INF,INF,INF,INF,INF,INF,INF,INF,0,INF,INF,INF,INF,INF,INF,INF,INF},
      {INF,INF,INF,INF,INF,INF,INF,INF,INF,INF,INF,INF,INF,343,INF,INF,INF,INF,INF,INF,557,INF,INF,INF,INF,INF,0,INF,INF,INF,INF,INF,INF,INF},
      {INF,INF,INF,INF,INF,240,INF,INF,INF,INF,INF,INF,INF,INF,INF,INF,INF,INF,271,INF,INF,INF,INF,INF,INF,INF,INF,0,INF,INF,INF,INF,INF,INF},
      {INF,INF,INF,565,648,INF,INF,INF,INF,INF,INF,INF,INF,INF,289,INF,INF,INF,INF,INF,INF,INF,INF,294,INF,INF,INF,INF,0,INF,639,INF,INF,INF},
      {INF,INF,INF,INF,INF,INF,INF,INF,467,INF,370,INF,INF,INF,INF,565,INF,INF,INF,374,358,INF,INF,467,430,INF,INF,INF,INF,0,INF,INF,INF,INF},//30
      {INF,261,INF,INF,324,INF,INF,INF,INF,INF,INF,INF,INF,766,INF,INF,INF,INF,INF,INF,INF,INF,INF,757,INF,INF,INF,INF,639,INF,0,INF,INF,INF},
      {INF,INF,255,INF,INF,INF,INF,INF,INF,INF,INF,INF,INF,INF,INF,INF,INF,INF,INF,INF,INF,INF,INF,INF,INF,INF,INF,INF,INF,INF,INF,0,INF,INF},
      {INF,INF,INF,133,INF,INF,INF,INF,INF,INF,INF,INF,INF,INF,INF,INF,INF,INF,INF,INF,INF,INF,INF,INF,INF,INF,INF,INF,INF,INF,INF,INF,0,INF},
      {INF,INF,INF,108,INF,INF,INF,INF,INF,INF,INF,INF,INF,INF,INF,INF,INF,INF,INF,INF,INF,INF,INF,INF,INF,INF,INF,INF,INF,INF,INF,INF,INF,0}};//34
QString Location[MAX_VERTEX_NUM] = {"北京","成都","福州","广州","贵阳","哈尔滨","海口","杭州","合肥","呼和浩特","济南","昆明","拉萨","兰州","南昌","南京","南宁","上海","沈阳","石家庄","太原","天津","乌鲁木齐","武汉","西安","西宁","银川","长春","长沙","郑州","重庆","台北","香港","澳门"};
QString LocationSpell[MAX_VERTEX_NUM] = {"bj","cd","fz","gz","gy","heb","hk","hz","hf","hhht","jn","km","ls","lz","nc","nj","nn","sh","sy","sjz","ty","tj","wlmq","wh","xa","xn","yc","cc","cs","zz","cq","tb","xg","am"};
//int nodeX[MAX_VERTEX_NUM] = {505,340,559,484,380,620,441,569,527,449,520,330,170,350,510,553,409,585,580,489,457,516,160,480,420,320,380,600,466,475,380,596,500,485};//节点的x坐标
//int nodeY[MAX_VERTEX_NUM] = {199,320,383,421,380,110,476,333,308,191,250,400,310,250,357,304,430,314,170,223,236,212,140,325,280,250,220,140,362,275,330,398,437,441};//节点的y坐标
// TODO 使用函数读取坐标后放入地图
int visited[MAX_VERTEX_NUM];//标志访问数组
/*--------------------------------------------------------------------------------*/
void ArrayToGvexs(QString vexs[MAX_VERTEX_NUM],MGraph *G);
void MatrixToGarcs(int M[][MAX_VERTEX_NUM],MGraph *G);
int FistAdjVex(MGraph G,int v);                             //返回v的第一个邻接顶点，没有则返回空
int NextAdjVex(MGraph G,int v,int w);                       //返回v相对于w的下一个顶点，若w是v的最后一个邻接点，则返回空
double ShortestPath_DIJ(MGraph G,int v0,int toNode[],int frontNode[]);
double ShortestPath_FLOYD(MGraph G,int dist[MAX_VERTEX_NUM][MAX_VERTEX_NUM]);
double ShortestPath_BFS(MGraph G,int v0,int v1,int toNode[],int frontNode[]);

void MatrixToGarcs(int M[][MAX_VERTEX_NUM],MGraph *G){//初始化邻接矩阵
    for(int i=0;i<MAX_VERTEX_NUM;i++){
        for(int j=0;j<MAX_VERTEX_NUM;j++){
                G->arcs[i][j].w = M[i][j];
        }
    }
}

int FistAdjVex(MGraph G,int v){//返回v的第一个邻接顶点，没有则返回空
    for(int i=0;i<MAX_VERTEX_NUM;i++)
        if(G.arcs[v][i].w>0&&G.arcs[v][i].w!=INF)
            return i;
    return -1;
}

int NextAdjVex(MGraph G,int v,int w){//返回v相对于w的下一个顶点，若w是v的最后一个邻接点，则返回空
    for(int i=w+1;i<MAX_VERTEX_NUM;i++)
        if(G.arcs[v][i].w>0&&G.arcs[v][i].w!=INF)
            return i;
    return -1;
}

double ShortestPath_DIJ(MGraph G,int from,int to,int toNode[],int frontNode[]){//图g，起始点v0，出发点到达当前点路径长度数组，前面点数组
    auto t1=std::chrono::steady_clock::now();
    /*------初始化------*/
    for(int v=0;v<MAX_VERTEX_NUM;++v){
        visited[v]=0;
        toNode[v]=INF;
        frontNode[v]=-1;
    }
    toNode[from]=0;//出发点到自己的路径长度为0
    for(int i=0;i<MAX_VERTEX_NUM;i++){
        /*------在未标记节点中选择距离出发点最近的节点，标记，收录进最优路径集合------*/
        int minLength=INF;
        int minNode=from;
        for(int j=0;j<MAX_VERTEX_NUM;j++){
            if(visited[j]==0&&toNode[j]<minLength){
                minLength=toNode[j];
                minNode=j;
            }
        }
        visited[minNode]=1;//标记已访问
        //if(minNode==to) break;//优化，标记终点退出循环，舍弃找到到其他点的最短路功能
        for(int w=FistAdjVex(G,minNode);w>=0;w=NextAdjVex(G,minNode,w)){
            if(!visited[w]){//该节点未访问，计算长度
                int tempLength = toNode[minNode]+MapRelation[minNode][w];
                if(tempLength<toNode[w]){//长度小于原先的长度则更新
                    toNode[w]=tempLength;
                    frontNode[w]=minNode;
                }
            }
        }
    }
    auto t2=std::chrono::steady_clock::now();
    qDebug()<<"本次算法执行耗时:"<<std::chrono::duration<double,std::milli>(t2-t1).count()<<"ms";
    return std::chrono::duration<double,std::milli>(t2-t1).count();
}

double ShortestPath_FLOYD(MGraph G,int dist[MAX_VERTEX_NUM][MAX_VERTEX_NUM]){//输入图G，dist数组
    auto t1=std::chrono::steady_clock::now();//计算开始时间
    for(int i=0;i<MAX_VERTEX_NUM;i++)
        for(int j=0;j<MAX_VERTEX_NUM;j++)
            dist[i][j] = G.arcs[i][j].w;//初始化
    for(int i=0;i<MAX_VERTEX_NUM;i++)//从j到k顶点只经过前i号点的最短路程
        for(int j=0;j<MAX_VERTEX_NUM;j++)
            for(int k=0;k<MAX_VERTEX_NUM;k++)
                dist[j][k] = std::min(dist[j][k],dist[j][i]+dist[i][k]);
    auto t2=std::chrono::steady_clock::now();//计算结束时间
    qDebug()<<"本次算法执行耗时:"<<std::chrono::duration<double,std::milli>(t2-t1).count()<<"ms";
    return std::chrono::duration<double,std::milli>(t2-t1).count();//返回耗时
}

double ShortestPath_BFS(MGraph G,int v0,int v1,int toNode[],int frontNode[]){//图g，起始点v0，终点v1，出发点到达当前点路径长度数组，前面点数组
    auto t1=std::chrono::steady_clock::now();
    for(int v=0;v<MAX_VERTEX_NUM;++v){
        visited[v]=0;
        toNode[v]=INF;
        frontNode[v]=-1;
    }//初始化
    SqQueue Q;
    InitQueue(&Q);
    EnQueue(&Q,v0);//初始节点入队
    toNode[v0] = 0;
    visited[v0] = 1;
    while(!QueueEmpty(Q)){
        int u;
        DeQueue(&Q,&u);
        if(u==v1){//已找到最短路
            break;
        }
        for(int w = FistAdjVex(G,u);w>=0;w=NextAdjVex(G,u,w))//访问下一层
            if(!visited[w]){
                toNode[w] = toNode[u] + G.arcs[w][u].w;//记录长度
                frontNode[w] = u;//记录前面点
                visited[w]=1;
                EnQueue(&Q,w);
            }
    }
    DestroyQueue(&Q);
    auto t2=std::chrono::steady_clock::now();
    qDebug()<<"本次算法执行耗时:"<<std::chrono::duration<double,std::milli>(t2-t1).count()<<"ms";
    return std::chrono::duration<double,std::milli>(t2-t1).count();
}
#endif // GRAPH_H
