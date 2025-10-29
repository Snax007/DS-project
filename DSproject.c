#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#define HASH_SIZE 100
#define MAX_NAME 50
#define MAX_DESC 100

typedef struct Product
 {
   int id;
    char name[MAX_NAME];
    int quantity;
    float price;
    char description[MAX_DESC];
}Product;

typedef struct HashNode 
 {
    Product product;
    struct HashNode* next;
}HashNode;

typedef struct 
{
    HashNode* table[HASH_SIZE];
}HashTable;

typedef struct AVLNode
  {
    Product product;
    struct AVLNode* left;
    struct AVLNode* right;
  int height;
}AVLNode;

typedef struct QueueNode 
 {
   int orderId;
    int productId;
    int quantity;
    char customerName[MAX_NAME];
    struct QueueNode* next;
}QueueNode;

typedef struct
 {
   QueueNode* front;
    QueueNode* rear;
    int count;
} Queue;

typedef struct PQNode
 {
    int orderId;
    int productId;
    int quantity;
    int priority; // 1=Urgent, 2=Express, 3=Normal
    char customerName[MAX_NAME];
    struct PQNode* next;
}PQNode;

typedef struct 
 {
    PQNode* head;
    int count;
}PriorityQueue;

HashTable* hashTable;
AVLNode* avlRoot = NULL;
Queue* deliveryQueue;
PriorityQueue* urgentQueue;
int nextOrderId = 1000;

int hashFunction(int id) 
 { 
 return id%HASH_SIZE; 

}

HashTable* createHashTable()
  {
    HashTable* ht=(HashTable*)malloc(sizeof(HashTable));
    for (int i=0;i<HASH_SIZE;i++) 
    {
        ht->table[i]=NULL;
    }
    return ht;
}

void hashInsert(HashTable* ht, Product p)
  {
    int index=hashFunction(p.id);
    HashNode* newNode=(HashNode*)malloc(sizeof(HashNode));
    newNode->product=p;
    newNode->next=ht->table[index];
    ht->table[index]=newNode;
}

Product* hashSearch(HashTable* ht, int id)
 {
    int index=hashFunction(id);
    HashNode* current=ht->table[index];
    while (current!=NULL) 
    {
        if (current->product.id==id)
        {
    return &(current->product);
}
     current=current->next;
    }
    return NULL;
}

int hashDelete(HashTable* ht,int id)
 {
    int index=hashFunction(id);
    HashNode* current=ht->table[index];
    HashNode* prev=NULL;

while(current!=NULL) 
  {
if (current->product.id==id)
 {
    if (prev==NULL)
    ht->table[index]=current->next;
else
 prev->next=current->next;
 free(current);
 return 1;
}
prev=current;
current=current->next;
}
    return 0;
}

void hashUpdate(HashTable* ht, int id, int quantity, float price)
 {
    Product* p=hashSearch(ht, id);
    if (p!=NULL)
 {
    p->quantity=quantity;
    p->price=price;
    }
}

int max(int a, int b) 
  { 
    if(a>b)
    return a;
    else
  return b;
}
int height(AVLNode* node) 
{
    if(node==NULL)
    return 0;
    else
    return node->height;
 }

int getBalance(AVLNode* node)
 {
    if (node==NULL) 
    return 0;
   return height(node->left)-height(node->right);
}

AVLNode* createAVLNode(Product p) 
 {
    AVLNode* node=(AVLNode*)malloc(sizeof(AVLNode));
    node->product=p;
    node->left=node->right = NULL;
    node->height=1;
    return node;
}

AVLNode* rightRotate(AVLNode* y) 
  {
    AVLNode* x=y->left;
    AVLNode* T2=x->right;
    x->right=y;
    y->left=T2;
    y->height=max(height(y->left),height(y->right))+1;
    x->height=max(height(x->left),height(x->right))+1;
    return x;
}

AVLNode* leftRotate(AVLNode* x) 
 {
    AVLNode* y=x->right;
    AVLNode* T2=y->left;
    y->left=x;
    x->right=T2;
    x->height=max(height(x->left),height(x->right))+1;
    y->height=max(height(y->left),height(y->right))+ 1;
   return y;
}

AVLNode* avlInsert(AVLNode* node,Product p) {
    if(node==NULL)
    return createAVLNode(p);

    if(p.id<node->product.id)
    node->left=avlInsert(node->left, p);

    else if (p.id>node->product.id)
    node->right=avlInsert(node->right, p);
    
    else
   return node;

    node->height=1+max(height(node->left),height(node->right));
    int balance=getBalance(node);

   if(balance>1 &&p.id<node->left->product.id)
    return rightRotate(node);
    if (balance<-1 && p.id>node->right->product.id)
   return leftRotate(node);
    if(balance>1 && p.id>node->left->product.id)
     {
    node->left=leftRotate(node->left);
    return rightRotate(node);
    }
    if (balance<-1 && p.id<node->right->product.id) 
     {
        node->right=rightRotate(node->right);
        return leftRotate(node);
    }
    return node;
}

AVLNode* minValueNode(AVLNode* node)
 {
    AVLNode* current=node;
    while (current->left!=NULL)
    {
    current = current->left;
}
    return current;
}

AVLNode* avlDelete(AVLNode* root,int id) {
    if (root==NULL)
        return root;

    if (id<root->product.id)
      root->left=avlDelete(root->left,id);
    else if (id>root->product.id)
   root->right = avlDelete(root->right, id);
    else
{
    if((root->left==NULL)||(root->right==NULL))
     {
    AVLNode* temp = root->left ? root->left : root->right;
    if(temp==NULL)
    {
    temp=root;
    root=NULL;
 } 
 else 
{
 *root = *temp;
    }
    free(temp);
 }
  else
 {
AVLNode* temp=minValueNode(root->right);
root->product=temp->product;
root->right=avlDelete(root->right,temp->product.id);
}
}

if(root==NULL)
  return root;

root->height=1 +max(height(root->left),height(root->right));
int balance=getBalance(root);

    
if(balance>1 && getBalance(root->left)>=0)
    return rightRotate(root);
    if(balance>1&& getBalance(root->left)<0)
 {
    root->left=leftRotate(root->left);
    return rightRotate(root);
    }
    if(balance<-1&& getBalance(root->right)<=0)
  return leftRotate(root);
    if(balance<-1 &&getBalance(root->right)>0) {
     root->right=rightRotate(root->right);
    return leftRotate(root);
}
    return root;
}

void inorderTraversal(AVLNode* root)
 {
   if(root!=NULL) 
{
 inorderTraversal(root->left);
 printf("ID: %-6d  Name: %-20s  Qty: %-6d  Price: %.2f\n",root->product.id, root->product.name, root->product.quantity, root->product.price);
 inorderTraversal(root->right);
 }
}

Queue* createQueue()
 {
    Queue* q=(Queue*)malloc(sizeof(Queue));
    q->front=q->rear = NULL;
    q->count=0;
    return q;
}

void enqueue(Queue* q,int productId,int quantity,char* customerName)
 {
    QueueNode* newNode=(QueueNode*)malloc(sizeof(QueueNode));
    newNode->orderId=nextOrderId++;
    newNode->productId=productId;
    newNode->quantity=quantity;
    strcpy(newNode->customerName,customerName);
   newNode->next=NULL;

if(q->rear==NULL)
  q->front=q->rear=newNode;
else 
{  
    q->rear->next = newNode;
    q->rear = newNode;
    }
    q->count++;
printf("\nDelivery order #%d added to queue.\n", newNode->orderId);
}

void dequeue(Queue* q) 
{
    if(q->front==NULL)
{
printf("\nNo delivery orders in queue.\n");
     return;
    }
    QueueNode* temp=q->front;
printf("\nProcessing Order #%d (Customer: %s, Product ID: %d, Quantity: %d)\n",temp->orderId, temp->customerName, temp->productId, temp->quantity);
q->front=q->front->next;
if(q->front==NULL)
q->rear=NULL;
    free(temp);
    q->count--;
}

void displayQueue(Queue* q)
 {
    if(q->front==NULL)
 {
    printf("\n No delivery orders.\n");
    return;
    }

QueueNode* current=q->front;
 printf("\nDELIVERY QUEUE:\n");
  while(current!=NULL)
 {
 printf("Order #%d - %s (Product ID: %d, Qty: %d)\n",current->orderId, current->customerName, current->productId, current->quantity);
 current = current->next;
    }
}

PriorityQueue* createPriorityQueue()
{
    PriorityQueue* pq=(PriorityQueue*)malloc(sizeof(PriorityQueue));
    pq->head=NULL;
   pq->count=0;
   return pq;
}

void priorityEnqueue(PriorityQueue* pq, int productId, int quantity, int priority, char* customerName)
 {
    PQNode* newNode=(PQNode*)malloc(sizeof(PQNode));
    newNode->orderId=nextOrderId++;
    newNode->productId=productId;
    newNode->quantity=quantity;
    newNode->priority=priority;
    strcpy(newNode->customerName,customerName);
   newNode->next = NULL;
    if (pq->head==NULL||priority<pq->head->priority) 
{
    newNode->next=pq->head;
     pq->head=newNode;
    } 
else
{
 PQNode* current=pq->head;
while (current->next != NULL && current->next->priority <=priority)
{
    current = current->next;
}
  newNode->next=current->next;
    current->next=newNode;
    }
    pq->count++;
    printf("\n Priority order #%d added (Priority %d)\n", newNode->orderId, priority);
}

void priorityDequeue(PriorityQueue* pq)
{
 if(pq->head==NULL)
  {
    printf("\n No urgent/express orders.\n");
    }

PQNode* temp=pq->head;
printf("\n→ Processing Priority %d Order #%d (Customer: %s, Product ID: %d, Qty: %d)\n",temp->priority, temp->orderId, temp->customerName, temp->productId, temp->quantity);
 pq->head=pq->head->next;
  free(temp);
    pq->count--;
}

void displayPriorityQueue(PriorityQueue* pq)
 {
    if (pq->head==NULL)
    {
     printf("\n No priority orders.\n");
    return;
    }

    PQNode* current=pq->head;
    printf("\n PRIORITY QUEUE:\n");
    while(current!=NULL) 
{
 printf("[Priority %d] Order #%d - %s (Product ID: %d, Qty: %d)\n",current->priority, current->orderId, current->customerName, current->productId, current->quantity);
 current = current->next;
}
}

void addProduct() 
 {
    Product p;
    printf("\nEnter Product ID: ");
    scanf("%d",&p.id);
    if(hashSearch(hashTable,p.id)) {
  printf(" Product already exists!\n");
        
    }

 printf("Enter Product Name: ");
 scanf(" %[^\n]", p.name);
 printf("Enter Quantity: ");
scanf("%d", &p.quantity);
printf("Enter Price: ");
scanf("%f", &p.price);
 printf("Enter Description: ");
scanf(" %[^\n]", p.description);

 hashInsert(hashTable, p);
avlRoot=avlInsert(avlRoot, p);
printf("\n Product added successfully\n");
}

void removeProduct()
 {
    int id;
    printf("\nEnter Product ID to remove: ");
    scanf("%d",&id);
    Product* p=hashSearch(hashTable, id);
if(!p)
{
 printf("✗ Product not found!\n");
  
    }
 hashDelete(hashTable, id);
avlRoot = avlDelete(avlRoot, id);
printf("\n Product removed successfully\n");
}

void searchProduct() {
    int id;
    printf("\nEnter Product ID: ");
    scanf("%d", &id);
    Product* p = hashSearch(hashTable, id);
    if (p)
        printf("ID: %d\nName: %s\nQty: %d\nPrice: %.2f\nDescription: %s\n",
               p->id, p->name, p->quantity, p->price, p->description);
    else
        printf("✗ Product not found!\n");
}

void updateProduct() {
    int id, newQty;
    float newPrice;
    printf("\nEnter Product ID: ");
    scanf("%d",&id);
    Product* p=hashSearch(hashTable, id);
if (!p)
{
printf(" Product not found!\n");
return;
}
printf("Enter new Quantity: ");
scanf("%d",&newQty);
printf("Enter new Price: ");
scanf("%f",&newPrice);
 hashUpdate(hashTable, id, newQty, newPrice);
printf("Product updated successfully!\n");
}

void displayAllProducts()
 {
    printf("\nALL PRODUCTS (Sorted by ID):\n");
if(avlRoot==NULL)
printf("No products available.\n");
else
inorderTraversal(avlRoot);
}

void cleanup() 
{
for(int i=0;i<HASH_SIZE;i++)
 {
 HashNode* current=hashTable->table[i];
while(current)
{
    HashNode* temp=current;
    current=current->next;
    free(temp);
 }
}
free(hashTable);
}

void displayMenu()
 {
    printf("\nC-VENTORY: INVENTORY MANAGEMENT SYSTEM\n");
    printf("1. Add Product\n2. Remove Product\n3. Search Product\n4. Update Product\n5. Display All Products\n6. Add Delivery Order\n7. Process Delivery Order\n8. View Delivery Queue\n9. Add Priority Order\n10. Process Priority Order\n11. View Priority Queue\n0. Exit\n");
    printf("Enter your choice: ");
}

int main() 
{
  hashTable = createHashTable();
  deliveryQueue = createQueue();
  urgentQueue = createPriorityQueue();
int choice;
 printf("WELCOME TO C-VENTORY\n");
do
{
 displayMenu();
scanf("%d", &choice);
 switch (choice)
  {
    case 1: 
    addProduct();
    break;
   case 2: 
    removeProduct();
    break;
    case 3:
    searchProduct();
    break;
    case 4:updateProduct();
     break;
    case 5: 
    displayAllProducts(); 
    break;
    case 6:
{
int id, qty; char name[MAX_NAME];
  printf("Enter Product ID: "); scanf("%d", &id);
printf("Enter Quantity: "); scanf("%d", &qty);
 printf("Enter Customer Name: "); scanf(" %[^\n]", name);
enqueue(deliveryQueue, id, qty, name);
 break;
}
case 7:
dequeue(deliveryQueue); 
 break;

case 8:
 displayQueue(deliveryQueue);
  break;

case 9:
 {
    int id, qty, priority; char name[MAX_NAME];
printf("Enter Product ID: "); scanf("%d", &id);
 printf("Enter Quantity: "); scanf("%d", &qty);
 printf("Enter Priority(1=Urgent,2=Express,3=Normal): "); 
 scanf("%d", &priority);
printf("Enter Customer Name: ");
 scanf(" %[^\n]", name);
priorityEnqueue(urgentQueue, id, qty, priority, name);
break;
 }
 case 10:
  priorityDequeue(urgentQueue);
 break;
            
 case 11:
  displayPriorityQueue(urgentQueue);
 break;
            
 case 0: 
printf("\nExiting Thank you\n"); cleanup(); 
break;
default: printf("Invalid choice!\n");
   }
    }
while (choice != 0);
return 0;
}
