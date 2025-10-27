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
} Product;


typedef struct HashNode 
{    Product product;
    struct HashNode* next;
} HashNode;

typedef struct 
{
    HashNode* table[HASH_SIZE];
} HashTable;

typedef struct AVLNode 
{
    Product product;
    struct AVLNode* left;
    struct AVLNode* right;
    int height;
} AVLNode;

typedef struct QueueNode 
{
    int orderId;
    int productId;
    int quantity;
    char customerName[MAX_NAME];
    struct QueueNode* next;
} QueueNode;

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
} PQNode;

typedef struct 
{
    PQNode* head;
    int count;
} PriorityQueue;

HashTable* hashTable;
AVLNode* avlRoot = NULL;
Queue* deliveryQueue;
PriorityQueue* urgentQueue;
int nextOrderId = 1000;

int hashFunction(int id) 
{
    return id % HASH_SIZE;
}

HashTable* createHashTable() 
{
    HashTable* ht =(HashTable*)malloc(sizeof(HashTable));
    for (int i=0;i<HASH_SIZE;i++) 
{
        ht->table[i] = NULL;
    }
    return ht;
}

void hashInsert(HashTable* ht,Product p) 
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
    while(current!=NULL) {
        if(current->product.id==id)
         {
            return&(current->product);
        }
        current=current->next;
    }
return NULL;
}

int hashDelete(HashTable* ht, int id) 
{
    int index=hashFunction(id);
    HashNode* current=ht->table[index];
    HashNode* prev=NULL;
    
    while(current!=NULL) 
{
        if(current->product.id==id)
{
            if(prev==NULL) 
            {
            ht->table[index]=current->next;
            } 
            else
            {
                prev->next=current->next;
            }
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
    Product* p=hashSearch(ht,id);
    if (p!=NULL) 
{
        p->quantity = quantity;
        p->price = price;
    }
}

int max(int a,int b)
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
    return node->height;
}

int getBalance(AVLNode* node) 
{
    if(node == NULL)
    return 0;

    return height(node->left)-height(node->right);
}

AVLNode* createAVLNode(Product p) 
{
   AVLNode* node=(AVLNode*)malloc(sizeof(AVLNode));
    node->product=p;
    node->left=NULL;
    node->right=NULL;
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
    y->height=max(height(y->left),height(y->right))+1;
    return y;
}

AVLNode* avlInsert(AVLNode* node, Product p)
 {
    if(node==NULL)
     {
        return createAVLNode(p);
    }
    
    if(p.id<node->product.id) 
{
        node->left=avlInsert(node->left,p);
    }
 else if(p.id> node->product.id) 
 {
        node->right = avlInsert(node->right, p);
    }
 else {
        return node; 
    }
    
    node->height=1+max(height(node->left),height(node->right));
    int balance=getBalance(node);
    
    if (balance>1&& p.id<node->left->product.id) 
{
        return rightRotate(node);
    }
   
    if (balance<-1 && p.id>node->right->product.id)
     {
        return leftRotate(node);
    }
    
    if (balance>1&&p.id>node->left->product.id) 
    {
        node->left = leftRotate(node->left);
        return rightRotate(node);
    }
    
   
    if (balance<-1&& p.id<node->right->product.id) 
    {
        node->right = rightRotate(node->right);
        return leftRotate(node);
    }
    
    return node;
}

AVLNode* minValueNode(AVLNode* node) 
{
    AVLNode* current=node;
    while(current->left!=NULL) 
    {
        current=current->left;
    }
    
return current;
}

AVLNode* avlDelete(AVLNode* root, int id) 
{
    if(root==NULL)
     return root;
    
    if(id<root->product.id) 
    {
        root->left=avlDelete(root->left,id);
    } 
    else if (id>root->product.id) 
    {
        root->right=avlDelete(root->right,id);
    } 
    else
     {
        if ((root->left==NULL)||(root->right==NULL))
     {
        AVLNode* temp=root->left?root->left:root->right;
        if(temp==NULL)
         {
            temp=root;
            root=NULL;
            } 
            else
            {
          *root=*temp;
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
    
    root->height=1+max(height(root->left),height(root->right));

    
    int balance=getBalance(root);
   
    if (balance>1&&getBalance(root->left)>=0)
     {
        return rightRotate(root);
    }
    
    if(balance>1&&getBalance(root->left)<0)
 {
        root->left=leftRotate(root->left);
        return rightRotate(root);
    }
    
    if(balance<-1 &&getBalance(root->right)<=0) 
    {
        return leftRotate(root);
    }
   
    if (balance<-1&& getBalance(root->right)>0)
 {
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
        printf("ID: %-6d \n Name: %-20s \n Qty: %-6d \n Price: %.2f\n",root->product.id, root->product.name, root->product.quantity, root->product.price);
    inorderTraversal(root->right);
    }
}

Queue* createQueue() 
{
  Queue* q=(Queue*)malloc(sizeof(Queue));
  q->front = NULL;
    q->rear = NULL;
    q->count = 0;
    return q;
}

void enqueue(Queue* q,int productId,int quantity,char* customerName) 
{
    QueueNode* newNode=(QueueNode*)malloc(sizeof(QueueNode));
    newNode->orderId=nextOrderId++;
    newNode->productId=productId;
    newNode->quantity= quantity;
    strcpy(newNode->customerName, customerName);
    newNode->next =NULL;
    
    if (q->rear==NULL) 
    {
        q->front=q->rear=newNode;
    } 
    else
     {
        q->rear->next=newNode;
        q->rear=newNode;
    }
    q->count++;
    printf("\n Delivery order #%d added to queue \n", newNode->orderId);
}

void dequeue(Queue* q) 
{
    if (q->front==NULL)
     {
        printf("\n No delivery orders in queue\n");
        return;
    }
    
    QueueNode*temp=q->front;
    printf("\n Processing Order #%d-Customer:%s,Product ID:%d,Quantity:%d\n",temp->orderId, temp->customerName, temp->productId, temp->quantity);
q->front = q->front->next;
    if (q->front==NULL) 
    {
    q->rear=NULL;
    }
    free(temp);
    q->count--;
}

void displayQueue(Queue* q) 
{
    if(q->front==NULL)
     {
        printf("\n No delivery orders in queue.\n");
        return;
    }
    
    printf("\n DELIVERY QUEUE\n");
    QueueNode*current=q->front;
    int pos=1;
    while(current!=NULL)
     {
        printf("%d Order #%d - Customer %s \n Product ID: %d \nQty: %d\n",pos++, current->orderId, current->customerName,current->productId, current->quantity);
        current = current->next;
    }
    printf("Total orders in queue: %d\n", q->count);
}

PriorityQueue* createPriorityQueue() 
 {
    PriorityQueue* pq=(PriorityQueue*)malloc(sizeof(PriorityQueue));
    pq->head=NULL;
  pq->count=0;
    return pq;
}

void priorityEnqueue(PriorityQueue* pq, int productId, int quantity,int priority, char* customerName) 
{
PQNode* newNode=(PQNode*)malloc(sizeof(PQNode));
    newNode->orderId=nextOrderId++;
    newNode->productId=productId;
    newNode->quantity=quantity;
    newNode->priority=priority;
   strcpy(newNode->customerName,customerName);
    newNode->next=NULL;
    
    if (pq->head==NULL||priority<pq->head->priority) 
    {
        newNode->next=pq->head;
        pq->head=newNode;
    } 
    else 
{
 PQNode* current = pq->head;
while(current->next != NULL && current->next->priority <= priority) 
    {
current = current->next;
}
newNode->next = current->next;
current->next = newNode;
}
pq->count++;
    
char* priorityStr[] = {"", "URGENT", "EXPRESS", "NORMAL"};
    printf("\n✓ %s order #%d added to priority queue!\n",priorityStr[priority], newNode->orderId);
}

void priorityDequeue(PriorityQueue* pq) 
{
    if (pq->head==NULL)
     {
        printf("\n No urgent/express orders!\n");
        return;
    }
    
PQNode* temp=pq->head;
    char* priorityStr[] = {"","URGENT","EXPRESS","NORMAL"};
    printf("\n✓ Processing %s Order #%d - Customer: %s, Product ID: %d, Quantity: %d\n", priorityStr[temp->priority], temp->orderId, temp->customerName,temp->productId, temp->quantity);
    
    pq->head = pq->head->next;
    free(temp);
    pq->count--;
}

void displayPriorityQueue(PriorityQueue* pq)
 {
    if (pq->head == NULL) {
        printf("\n✗ No urgent/express orders.\n");
        return;
    }
    
    printf("\n PRIORITY QUEUE (URGENT/EXPRESS ORDERS)\n");
    PQNode* current=pq->head;
    int pos=1;
    char*priorityStr[]={"", "URGENT", "EXPRESS", "NORMAL"};
    while (current!=NULL)
     {
        printf("%d. [%s] Order #%d - Customer: %s \nProduct ID: %d \n Qty: %d\n",pos++, priorityStr[current->priority], current->orderId,current->customerName, current->productId, current->quantity);
        current = current->next;
    }
    printf("Total priority orders: %d\n", pq->count);
}

void addProduct() 
{
    Product p;
    printf("\nADD NEW PRODUCT\n");
    printf("Enter Product ID: ");
    scanf("%d", &p.id);
    
    if (hashSearch(hashTable, p.id) != NULL) {
        printf("\n✗ Product ID already exists!\n");
  return;
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
    avlRoot = avlInsert(avlRoot, p);
    
    printf("\nProduct added successfully!\n");
}

void removeProduct() 
{
    int id;
    printf("\n REMOVE PRODUCT\n");
    printf("Enter Product ID to remove:");
    scanf("%d",&id);
    Product* p=hashSearch(hashTable, id);
    if (p==NULL)
    {
printf("\n Product not found!\n");
    return;
    }
    
    printf("Product: %s (Qty: %d, Price: %.2f)\n", p->name, p->quantity, p->price);
    printf("Are you sure you want to remove this product? (1=Yes, 0=No): ");
  int confirm;
   scanf("%d", &confirm);
    if(confirm==1)
     {
        hashDelete(hashTable,id);
        avlRoot = avlDelete(avlRoot,id);
        printf("\n Product removed successfully!\n");
    } else
     {
        printf("\n Removal cancelled.\n");
    }
}

void searchProduct()
 {
    int id;
    printf("\n SEARCH PRODUCT\n");
    printf("Enter Product ID: ");
    scanf("%d", &id);
 Product* p = hashSearch(hashTable, id);
    if(p!=NULL) 
    {
        printf("\n Product Found\n");
        printf("ID: %d\n", p->id);
        printf("Name: %s\n", p->name);
        printf("Quantity: %d\n", p->quantity);
        printf("Price: $%.2f\n", p->price);
        printf("Description: %s\n", p->description);
    }
     else 
{
        printf("\n Product not found!\n");
    }
}

void updateProduct()
 {
    int id;
    printf("\n UPDATE PRODUCT\n");
    printf("Enter Product ID: ");
   scanf("%d",&id);
    
    Product* p=hashSearch(hashTable, id);
    if(p==NULL) 
    {
        printf("\n Product not found!\n");
     return;
    }
    
 printf("Current Details - Name: %s, Qty: %d, Price: $%.2f\n", p->name, p->quantity, p->price);
    
int newQty;
float newPrice;
    printf("Enter new Quantity: "); 
 scanf("%d",&newQty);
    printf("Enter new Price: ");
    scanf("%f",&newPrice);
    
  hashUpdate(hashTable,id,newQty,newPrice);
    printf("\nProduct updated successfully!\n");
}

void displayAllProducts() 
  {
    printf("\n ALL PRODUCTS (SORTED BY ID)\n");
    if (avlRoot==NULL) {
    printf(" No products in inventory.\n");
        return;
    }
    inorderTraversal(avlRoot);
}

void lowStockAlert()
 {
    int threshold;
    printf("\n LOW STOCK ALERT \n");
    printf("Enter threshold quantity: ");
    scanf("%d", &threshold);
    
    printf("\n Products Below Threshold\n");
    int found=0;
    for (int i=0;i<HASH_SIZE;i++) 
{
    HashNode* current = hashTable->table[i];
    while(current!=NULL) 
    {
       if(current->product.quantity<threshold) 
    {
     printf("ID: %d \n Name: %s\n Qty: %d\n Price: $%.2f\n",
    current->product.id, current->product.name,
 current->product.quantity, current->product.price);
        found = 1;
            }
            current=current->next;
        }
    }
    if (!found) 
{
        printf(" All products are sufficiently stocked!\n");
    }
}
void addDeliveryOrder()
 {
    int productId,quantity;
    char customerName[MAX_NAME];
    
    printf("\n ADD DELIVERY ORDER\n");
    printf("Enter Product ID: ");
    scanf("%d", &productId);
    
    Product* p = hashSearch(hashTable, productId);
    if (p==NULL) 
{
        printf("\n Product not found!\n");
        return;
    }
    
  printf("Product: %s (Available: %d)\n", p->name, p->quantity);
    printf("Enter Quantity: ");
    scanf("%d", &quantity);
    
    if (quantity>p->quantity) 
{
        printf("\n✗ Insufficient stock!\n");
        return;
    }
    
    printf("Enter Customer Name: ");
    scanf(" %[^\n]",customerName);
    
    enqueue(deliveryQueue, productId, quantity, customerName);
}

void addPriorityOrder() 
{
    int productId, quantity, priority;
    char customerName[MAX_NAME];
 printf("\n ADD PRIORITY ORDER \n");
    printf("Enter Product ID: ");
    scanf("%d", &productId);
    
    Product* p=hashSearch(hashTable, productId);
    if (p==NULL) 
{
        printf("\n Product not found!\n");
        return;
    }
    
    printf("Product: %s (Available: %d)\n", p->name, p->quantity);
    printf("Enter Quantity: ");
    scanf("%d", &quantity);
    
    if (quantity > p->quantity) 
{
    printf("\n✗ Insufficient stock!\n");
   return;
    }
printf("Enter Priority (1=Urgent, 2=Express, 3=Normal): ");
    scanf("%d", &priority);
 if(priority<1||priority>3) 
{
printf("\n✗ Invalid priority!\n");
    return;
    }
 printf("Enter Customer Name: ");
    scanf(" %[^\n]", customerName);
priorityEnqueue(urgentQueue, productId, quantity, priority, customerName);
}

void saveToFile() {
    FILE* fp=fopen("inventory.dat", "wb");
    if(fp==NULL) 
{
printf("\nError opening file!\n");
return;
    }
    
    int count=0;
    for(int i=0;i<HASH_SIZE;i++) 
    {
    HashNode* current = hashTable->table[i];
        
    while(current!=NULL) 
    {
    fwrite(&(current->product),sizeof(Product), 1, fp);
     count++;
  current=current->next;
        }
}
    
    fclose(fp);
    printf("\n Inventory saved to file(%d products)\n", count);
}

void loadFromFile() {
    FILE* fp=fopen("inventory.dat","rb");
    if (fp==NULL) 
{
        printf("\nNo saved inventory file found.\n");
        return;
    }
    
    Product p;
    int count=0;
    while(fread(&p, sizeof(Product),1,fp)==1)
     {
hashInsert(hashTable, p);
    avlRoot = avlInsert(avlRoot, p);
        count++;
    }
    
    fclose(fp);
    printf("\n Inventory loaded from file( %dproducts)\n",count);
}

void displayMenu() {
    printf("\n");
    printf("C-VENTORY: INVENTORY MANAGEMENT SYSTEM \n");
    printf("\n PRODUCT MANAGEMENT\n");
    printf("1.Add Product\n");
    printf("2.Remove Product\n");
    printf("3. Search Product (Hash Table - O(1))\n");
    printf("4. Update Product\n");
    printf("5.  Display All Products (AVL Tree - Sorted)\n");
    printf("6.  Low Stock Alert\n");
    printf("\nORDER MANAGEMENT\n");
    printf("7. Add Delivery Order\n");
    printf("8.Process Next Delivery Order\n");
    printf("9.View Delivery Queue\n");
    printf("10. Add Priority Order (Urgent/Express)\n");
    printf("11. Process Next Priority Order\n");
    printf("12. View Priority Queue\n");
    printf("\n FILE OPERATIONS\n");
    printf("13. Save Inventory to File\n");
    printf("14. Load Inventory from File\n");
    printf("\n0.  Exit\n");
    printf("\nEnter your choice: ");
}

void initialize() {
    hashTable = createHashTable();
    deliveryQueue = createQueue();
    urgentQueue = createPriorityQueue();
}

void cleanup() 
{
    for(int i=0;i<HASH_SIZE;i++) 
{
        HashNode* current=hashTable->table[i];
        while(current!=NULL) 
    {
            HashNode* temp=current;
            current=current->next;
            free(temp);
        }
    }
    free(hashTable);
    
    while(deliveryQueue->front!=NULL)
     {
        QueueNode* temp=deliveryQueue->front;
        deliveryQueue->front=deliveryQueue->front->next;
        free(temp);
    }
free(deliveryQueue);
    
    while(urgentQueue->head!=NULL)
     {
        PQNode*temp=urgentQueue->head;
        urgentQueue->head=urgentQueue->head->next;
        free(temp);
    }
    free(urgentQueue);
}

int main() {
    initialize();
    int choice;
    printf(" WELCOME TO C-VENTORY SYSTEM \n");
    printf("Advanced Inventory Management Solution \n");
    
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
    case 4:
 updateProduct();
break;
    case 5:
displayAllProducts();
break;
    case 6:
lowStockAlert();
break;
    case 7:
 addDeliveryOrder();
 break;
    case 8:
dequeue(deliveryQueue);
break;
    case 9:
displayQueue(deliveryQueue);
break;
    case 10:
addPriorityOrder();
break;
    case 11:
priorityDequeue(urgentQueue);
break;
    case 12:
displayPriorityQueue(urgentQueue);
 break;
    case 13:
 saveToFile(); 
break;
    case 14:
loadFromFile();
 break;
    case 0:
 printf("\nSaving and exiting\n");
  saveToFile();
cleanup();
printf("Thank you for using C-ventory\n");
break;
 default:
printf("\n Invalid choice! Please try again.\n");
        }
        if (choice!=0) 
        {
        printf("\nPress Enter to continue");
     getchar();
     getchar();
    }
}while (choice != 0);
return 0;
}