#ifndef queue_Sq
#define queue_Sq
#include <malloc.h>
#define TRUE 1
#define FALSE 0
#define OK 1
#define ERROR 0
#define OVERFLOW -2
#define QUEUE_INIT_SIZE 50	//动态数组初始空间
#define QUEUEINCREMENT 100	//动态数组空间增量

#ifndef QELEMTYPE
#define QELEMTYPE
typedef int QElemType;
#endif

typedef int Status;
typedef struct{
    QElemType *base;
    int front;
    int rear;
    int size;
}SqQueue;

Status InitQueue(SqQueue *Q);     //创建空队列Q
Status DestroyQueue(SqQueue *Q);  //销毁队列Q
Status ClearQueue(SqQueue *Q);    //清空队列Q
Status QueueEmpty(SqQueue Q);     //若队列空，返回TRUE，否则返回FALSE
int QueueLength(SqQueue Q);       //返回Q的元素个数
Status GetHead(SqQueue Q,QElemType *e);   //若队Q不空，则用e返回Q的队头元素，返回OK，否则返回ERROR
Status EnQueue(SqQueue *Q,QElemType e);   //将e插入Q队尾
Status DeQueue(SqQueue *Q,QElemType *e);  //若队Q不空，删除Q队头元素，用e返回，返回OK，否则返回ERROR
Status QueueTraverse(SqQueue Q,Status (*visit)(QElemType *e));//依次对Q的每个元素调用visit(),一旦visit()调用失败,则操作失败

Status InitQueue(SqQueue *Q){//创建空队列Q
    Q->base=(QElemType*)malloc(QUEUE_INIT_SIZE*sizeof(QElemType));
    if(!Q->base){
        exit(OVERFLOW);
    }else{
        Q->front=Q->rear=0;
        Q->size=QUEUE_INIT_SIZE;
        return OK;
    }
}

Status DestroyQueue(SqQueue *Q){//销毁队列Q
    Q->front=Q->rear=0;
    free(Q->base);
    return OK;
}

Status ClearQueue(SqQueue *Q){//清空队列Q
    Q->front=Q->rear=0;
    return OK;
}

Status QueueEmpty(SqQueue Q){//若队列空，返回TRUE，否则返回FALSE
    if(Q.front==Q.rear){
        return TRUE;
    }else{
        return FALSE;
    }
}

int QueueLength(SqQueue Q){//返回Q的元素个数
    return (Q.rear-Q.front+Q.size)%Q.size;
}

Status GetHead(SqQueue Q,QElemType *e){//若队Q不空，则用e返回Q的队头元素，返回OK，否则返回ERROR
    if(Q.front==Q.rear){
        return ERROR;
    }else{
        *e=Q.base[Q.front];
        return OK;
    }
}

Status EnQueue(SqQueue *Q,QElemType e){//将e插入Q队尾
    if((Q->rear+1)%(Q->size)==Q->front){//队满
        QElemType *newbase = (QElemType*)realloc(Q->base,Q->size + QUEUEINCREMENT*sizeof(QElemType));
		if(!newbase){//分配失败
			exit(OVERFLOW);
		}
		Q->base = newbase;
        Q->size=Q->size+QUEUEINCREMENT;//增加空间
    }
    Q->base[Q->rear]=e;
    Q->rear=(Q->rear+1)%Q->size;
    return OK;
}

Status DeQueue(SqQueue *Q,QElemType *e){//若队Q不空，删除Q队头元素，用e返回，返回OK，否则返回ERROR
    if(Q->front==Q->rear){
        return ERROR;
    }else{
        *e=Q->base[Q->front];
        Q->front=(Q->front+1)%Q->size;//头后移
        return OK;
    }
}

Status QueueTraverse(SqQueue Q,Status (*visit)(QElemType *e)){//依次对Q的每个元素调用visit(),一旦visit()调用失败,则操作失败
    QElemType *p=Q.base;
    int length = (Q.front+QueueLength(Q))%Q.size;
	for(int i=Q.front;i<length;i++){
		if(!visit(p+i)){
			return ERROR;
		}
	}
	return OK;
}
#endif
