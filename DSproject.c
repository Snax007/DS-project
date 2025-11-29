#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_NAME 50
#define MAX_DESC 100
#define FILE_INVENTORY "inventory.txt"
#define FILE_DELIVERIES "deliveries.txt"
#define FILE_USERS "users.txt"
#define LOW_STOCK_LIMIT 5   // Threshold for low stock warning

// ---------- PRODUCT ----------
typedef struct {
    int id;
    char name[MAX_NAME];
    int quantity;
    float price;
    char description[MAX_DESC];
} Product;

// ---------- AVL NODE ----------
typedef struct AVLNode {
    Product product;
    struct AVLNode* left;
    struct AVLNode* right;
    int height;
} AVLNode;

// ---------- QUEUE ----------
typedef struct QueueNode {
    int orderId;
    int productId;
    int quantity;
    char customerName[MAX_NAME];
    struct QueueNode* next;
} QueueNode;

typedef struct {
    QueueNode* front;
    QueueNode* rear;
    int count;
} Queue;

int nextOrderId = 1000;

// ---------- AUTH ----------
typedef struct {
    char username[MAX_NAME];
    char password[100]; // plaintext here; replace with hash in production
    char role[10]; // "admin" or "staff"
} User;

int isLoggedIn = 0;
char currentUser[MAX_NAME];
char currentRole[10];

void strip_newline(char *s){
    size_t len = strlen(s);
    if(len>0 && s[len-1]=='\n') s[len-1]='\0';
}

// Check credentials in users file (plaintext check for demo)
int authenticateUser(const char* uname, const char* pwd, char* outRole){
    FILE* f = fopen(FILE_USERS, "r");
    if(!f) return 0;
    char line[300];
    while(fgets(line, sizeof(line), f)){
        strip_newline(line);
        if(strlen(line)==0) continue;
        char *u = strtok(line, ",");
        char *p = strtok(NULL, ",");
        char *r = strtok(NULL, ",");
        if(!u || !p || !r) continue;
        if(strcmp(u, uname)==0 && strcmp(p, pwd)==0){
            strcpy(outRole, r);
            fclose(f);
            return 1;
        }
    }
    fclose(f);
    return 0;
}

// Append a new user to file. Caller should ensure only admin calls this.
int registerUserToFile(const char* uname, const char* pwd, const char* role){
    // Check if user exists
    FILE* f = fopen(FILE_USERS, "r");
    if(f){
        char line[300];
        while(fgets(line, sizeof(line), f)){
            char tmp[300]; strcpy(tmp, line);
            char *u = strtok(tmp, ",");
            if(u && strcmp(u, uname)==0){
                fclose(f);
                return 0; // user exists
            }
        }
        fclose(f);
    }
    f = fopen(FILE_USERS, "a");
    if(!f) return 0;
    // NOTE: In production, store hashed password instead of plaintext!
    fprintf(f, "%s,%s,%s\n", uname, pwd, role);
    fclose(f);
    return 1;
}

// ---------- AVL UTILITIES ----------
int height(AVLNode* n){ return n ? n->height : 0; }
int max(int a,int b){ return a>b?a:b; }

AVLNode* createNode(Product p){
    AVLNode* n = malloc(sizeof(AVLNode));
    n->product = p;
    n->left = n->right = NULL;
    n->height = 1;
    return n;
}

AVLNode* rightRotate(AVLNode* y){
    AVLNode* x = y->left; AVLNode* T2 = x->right;
    x->right = y; y->left = T2;
    y->height = max(height(y->left),height(y->right))+1;
    x->height = max(height(x->left),height(x->right))+1;
    return x;
}

AVLNode* leftRotate(AVLNode* x){
    AVLNode* y = x->right; AVLNode* T2 = y->left;
    y->left = x; x->right = T2;
    x->height = max(height(x->left),height(x->right))+1;
    y->height = max(height(y->left),height(y->right))+1;
    return y;
}

int getBalance(AVLNode* n){ return n?height(n->left)-height(n->right):0; }

AVLNode* insertAVL(AVLNode* root, Product p){
    if(!root) return createNode(p);
    if(p.id < root->product.id) root->left = insertAVL(root->left,p);
    else if(p.id > root->product.id) root->right = insertAVL(root->right,p);
    else{ printf("Product ID already exists!\n"); return root; }

    root->height = 1+max(height(root->left),height(root->right));
    int bal = getBalance(root);

    if(bal>1 && p.id<root->left->product.id) return rightRotate(root);
    if(bal<-1 && p.id>root->right->product.id) return leftRotate(root);
    if(bal>1 && p.id>root->left->product.id){ root->left=leftRotate(root->left); return rightRotate(root);}
    if(bal<-1 && p.id<root->right->product.id){ root->right=rightRotate(root->right); return leftRotate(root);}
    return root;
}

AVLNode* minValueNode(AVLNode* n){ while(n->left) n=n->left; return n; }

AVLNode* deleteAVL(AVLNode* root,int id){
    if(!root) return root;
    if(id<root->product.id) root->left = deleteAVL(root->left,id);
    else if(id>root->product.id) root->right = deleteAVL(root->right,id);
    else{
        if(!root->left||!root->right){
            AVLNode* tmp = root->left?root->left:root->right;
            free(root); return tmp;
        }
        AVLNode* tmp = minValueNode(root->right);
        root->product = tmp->product;
        root->right = deleteAVL(root->right,tmp->product.id);
    }

    root->height = 1+max(height(root->left),height(root->right));
    int bal = getBalance(root);
    if(bal>1 && getBalance(root->left)>=0) return rightRotate(root);
    if(bal>1 && getBalance(root->left)<0){ root->left=leftRotate(root->left); return rightRotate(root);}
    if(bal<-1 && getBalance(root->right)<=0) return leftRotate(root);
    if(bal<-1 && getBalance(root->right)>0){ root->right=rightRotate(root->right); return leftRotate(root);}
    return root;
}

AVLNode* searchAVL(AVLNode* root,int id){
    if(!root) return NULL;
    if(id==root->product.id) return root;
    return id<root->product.id?searchAVL(root->left,id):searchAVL(root->right,id);
}

void inorderDisplay(AVLNode* root){
    if(!root) return;
    inorderDisplay(root->left);
    printf("ID:%-5d | %-20s | Qty:%-4d | Rs.%-8.2f | %s",
           root->product.id,root->product.name,root->product.quantity,
           root->product.price,root->product.description);
    if(root->product.quantity<=LOW_STOCK_LIMIT)
        printf(" Low Stock!\n");
    else printf("\n");
    inorderDisplay(root->right);
}

// ---------- FILE HANDLING ----------
void saveToFile(AVLNode* root, FILE* f){
    if(!root) return;
    saveToFile(root->left,f);
    fprintf(f,"%d,%s,%d,%.2f,%s\n",root->product.id,root->product.name,
            root->product.quantity,root->product.price,root->product.description);
    saveToFile(root->right,f);
}

void saveInventory(AVLNode* root){
    FILE* f=fopen(FILE_INVENTORY,"w");
    if(!f){ printf("Error saving inventory!\n"); return; }
    saveToFile(root,f);
    fclose(f);
}

AVLNode* loadFromFile(){
    FILE* f=fopen(FILE_INVENTORY,"r");
    if(!f) return NULL;
    AVLNode* root=NULL; Product p;
    while(fscanf(f,"%d,%49[^,],%d,%f,%99[^\n]\n",
                 &p.id,p.name,&p.quantity,&p.price,p.description)==5){
        root=insertAVL(root,p);
    }
    fclose(f);
    return root;
}

// ---------- QUEUE ----------
Queue* createQueue(){ Queue* q=malloc(sizeof(Queue)); q->front=q->rear=NULL; q->count=0; return q; }

void enqueue(Queue* q,int pid,int qty,const char* cname){
    QueueNode* n=malloc(sizeof(QueueNode));
    n->orderId=nextOrderId++; n->productId=pid; n->quantity=qty;
    strcpy(n->customerName,cname); n->next=NULL;
    if(!q->rear) q->front=q->rear=n;
    else{ q->rear->next=n; q->rear=n; }
    q->count++;

    FILE* f=fopen(FILE_DELIVERIES,"a");
    if(f){ fprintf(f,"%d,%s,%d,%d\n",n->orderId,n->customerName,n->productId,n->quantity); fclose(f); }

    printf("Order #%d added for %s\n",n->orderId,cname);
}

void dequeue(Queue* q,AVLNode* root){
    if(!q->front){ printf("No pending deliveries.\n"); return; }
    QueueNode* t=q->front;
    AVLNode* p=searchAVL(root,t->productId);
    if(!p) printf("Product %d not found.\n",t->productId);
    else if(p->product.quantity<t->quantity)
        printf("Insufficient stock for %s (Need:%d, Have:%d)\n",
               p->product.name,t->quantity,p->product.quantity);
    else{
        p->product.quantity -= t->quantity;
        printf("→ Delivered Order #%d | %s | %s x%d | Remaining:%d\n",
               t->orderId,t->customerName,p->product.name,t->quantity,p->product.quantity);
        if(p->product.quantity<=LOW_STOCK_LIMIT)
            printf("Low stock alert for %s! Only %d left.\n",
                   p->product.name,p->product.quantity);
        saveInventory(root);   // auto-save after each delivery
    }
    q->front=q->front->next;
    if(!q->front) q->rear=NULL;
    free(t); q->count--;
}

void displayQueue(Queue* q){
    if(!q->front){ printf("No pending deliveries.\n"); return; }
    printf("\nPending Deliveries:\n");
    QueueNode* t=q->front;
    while(t){
        printf("Order#%d | %s | ProductID:%d | Qty:%d\n",
               t->orderId,t->customerName,t->productId,t->quantity);
        t=t->next;
    }
}

// ---------- AUTH UI ----------
void authMenu(){
    int aChoice;
    while(!isLoggedIn){
        printf("\n=== LOGIN MENU ===\n1.Login\n2.Register (admin only)\n0.Exit\n> ");
        scanf("%d",&aChoice);
        if(aChoice==1){
            char uname[MAX_NAME], pwd[100], roleOut[10];
            printf("Username: "); scanf(" %49s", uname);
            printf("Password: "); scanf(" %99s", pwd);
            if(authenticateUser(uname, pwd, roleOut)){
                isLoggedIn = 1;
                strcpy(currentUser, uname);
                strcpy(currentRole, roleOut);
                printf("✓ Logged in as %s (%s)\n", currentUser, currentRole);
                return;
            } else {
                printf("✗ Invalid credentials.\n");
            }
        } else if(aChoice==2){
            // To register we require an admin to already exist or first-time setup.
            // If users file is empty: allow creation of first admin.
            FILE* f = fopen(FILE_USERS, "r");
            int users_exist = (f != NULL);
            if(f) fclose(f);

            if(!users_exist){
                // First admin creation
                char uname[MAX_NAME], pwd[100];
                printf("No users found. Create initial admin account.\n");
                printf("New admin username: "); scanf(" %49s", uname);
                printf("New admin password: "); scanf(" %99s", pwd);
                if(registerUserToFile(uname, pwd, "admin")) printf("Admin created. Please login.\n");
                else printf("Failed to create admin.\n");
            } else {
                // Regular register: allow only when an admin is logged in.
                if(!isLoggedIn || strcmp(currentRole, "admin")!=0){
                    printf("Only an admin can register new users. Please ask an admin to create the account or login as admin.\n");
                } else {
                    char uname[MAX_NAME], pwd[100], role[10];
                    printf("New username: "); scanf(" %49s", uname);
                    printf("Password: "); scanf(" %99s", pwd);
                    printf("Role (admin/staff): "); scanf(" %9s", role);
                    if(strcmp(role,"admin")!=0 && strcmp(role,"staff")!=0){
                        printf("Invalid role. Must be 'admin' or 'staff'.\n");
                    } else {
                        if(registerUserToFile(uname, pwd, role)) printf("User %s created with role %s.\n", uname, role);
                        else printf("User already exists or error.\n");
                    }
                }
            }
        } else if(aChoice==0){
            printf("Goodbye!\n"); exit(0);
        } else printf("Invalid choice.\n");
    }
}

// ---------- MAIN ----------
int main(){
    AVLNode* root=loadFromFile();
    Queue* dq=createQueue();

    printf("\n=== C-VENTORY ADVANCED INVENTORY SYSTEM ===\n");

    // Authenticate before showing main menu
    authMenu();

    int ch;
    do{
        printf("\n1.Add Product\n2.Remove Product\n3.Search Product\n4.Update Product\n");
        printf("5.Display All\n6.Add Delivery\n7.Process Delivery\n8.View Queue\n9.User Management (admin)\n10.Logout\n0.Exit\n> ");
        scanf("%d",&ch);

        if(ch==1){
            // Only staff or admin can add product (both allowed)
            Product p;
            printf("Enter ID: "); scanf("%d",&p.id);
            printf("Enter Name: "); scanf(" %49[^\n]",p.name);
            printf("Enter Qty: "); scanf("%d",&p.quantity);
            printf("Enter Price: "); scanf("%f",&p.price);
            printf("Enter Desc: "); scanf(" %99[^\n]",p.description);
            root=insertAVL(root,p);
            saveInventory(root);
            if(p.quantity<=LOW_STOCK_LIMIT)
                printf(" Low stock alert! Only %d units of %s\n",p.quantity,p.name);
        }
        else if(ch==2){
            // Only admin allowed to delete products
            if(strcmp(currentRole,"admin")!=0){
                printf("Only admin can delete products.\n");
            } else {
                int id; printf("Enter ID to delete: "); scanf("%d",&id);
                root=deleteAVL(root,id); saveInventory(root);
                printf("Product deleted (if existed).\n");
            }
        }
        else if(ch==3){
            int id; printf("Enter ID to search: "); scanf("%d",&id);
            AVLNode* n=searchAVL(root,id);
            if(n){
                printf("%s | Qty:%d | Rs:%.2f | %s\n",
                       n->product.name,n->product.quantity,
                       n->product.price,n->product.description);
                if(n->product.quantity<=LOW_STOCK_LIMIT)
                    printf("Low stock!\n");
            } else printf("Not found.\n");
        }
        else if(ch==4){
            int id; printf("Enter ID to update: "); scanf("%d",&id);
            AVLNode* n=searchAVL(root,id);
            if(!n) printf("Not found.\n");
            else{
                printf("New Qty: "); scanf("%d",&n->product.quantity);
                printf("New Price: "); scanf("%f",&n->product.price);
                saveInventory(root);
                printf("Updated.\n");
                if(n->product.quantity<=LOW_STOCK_LIMIT)
                    printf("Low stock alert! Only %d left.\n",n->product.quantity);
            }
        }
        else if(ch==5) inorderDisplay(root);
        else if(ch==6){
            int pid,qty; char cname[MAX_NAME];
            printf("Product ID: "); scanf("%d",&pid);
            AVLNode* prod = searchAVL(root,pid);
            if(!prod){
                printf("Cannot add delivery — Product ID %d not found in inventory.\n", pid);
                continue;
            }
            printf("Qty: "); scanf("%d",&qty);
            if(qty > prod->product.quantity){
                printf("✗ Insufficient stock! Available: %d, Requested: %d\n", prod->product.quantity, qty);
                continue;
            }
            printf("Customer: "); scanf(" %49[^\n]",cname);
            enqueue(dq,pid,qty,cname);
        }
        else if(ch==7) dequeue(dq,root);
        else if(ch==8) displayQueue(dq);
        else if(ch==9){
            // Admin-only user management menu
            if(strcmp(currentRole,"admin")!=0){
                printf("Only admin can access user management.\n");
            } else {
                int um;
                printf("\nUser Management:\n1.Create User\n0.Back\n> ");
                scanf("%d",&um);
                if(um==1){
                    char uname[MAX_NAME], pwd[100], role[10];
                    printf("New username: "); scanf(" %49s", uname);
                    printf("Password: "); scanf(" %99s", pwd);
                    printf("Role (admin/staff): "); scanf(" %9s", role);
                    if(strcmp(role,"admin")!=0 && strcmp(role,"staff")!=0) printf("Invalid role.\n");
                    else if(registerUserToFile(uname, pwd, role)) printf("User created.\n");
                    else printf("User exists or error.\n");
                }
            }
        }
        else if(ch==10){
            // Logout: reset state and force re-authentication
            isLoggedIn = 0;
            currentUser[0] = '\0';
            currentRole[0] = '\0';
            printf("Logged out.\n");
            authMenu();
        }
        else if(ch==0){ saveInventory(root); printf("✓ Data saved. Goodbye!\n"); }
        else printf("Invalid choice!\n");
    }while(ch!=0);

    return 0;
}
