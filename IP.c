#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_ALIAS_LENGTH 10
#define MAX_LINE_LENGTH 50

// Structure for AVL tree nodes
typedef struct Node {
    char ip[16];
    char alias[MAX_ALIAS_LENGTH + 1];
    int height;
    struct Node *left;
    struct Node *right;
    struct Node *parent;
} Node;

// Global root for the AVL tree
Node *root = NULL;

// Function prototypes
int validateIP(char *ip);
int validateAlias(char *alias);
int isDuplicate(Node *node, char *ip, char *alias);
Node *insert(Node *node, char *ip, char *alias);
Node *rebalance(Node *node);
Node *rotateLeft(Node *node);
Node *rotateRight(Node *node);
Node *rotateLeftThenRight(Node *node);
Node *rotateRightThenLeft(Node *node);
int getHeight(Node *node);
int getBalanceFactor(Node *node);
void updateHeight(Node *node);
void parseFile(const char *filename);
void addAddress();
void lookUpAddress();
void updateAddress();
void deleteAddress();
Node *deleteNode(Node *node, const char *alias);
void displayList(Node *node);
void displayAliasesForLocation();
void displayErrorLog();
void freeTree(Node *node);
Node *findNodeByIP(Node *node, const char *ip);
Node *findNodeByAlias(Node *node, const char *alias);
int isValidIPv4Prefix(const char *prefix);
int containsUppercase(const char *alias);
void displayAliasesForLocationHelper(Node *node, const char *prefix, int *found);
int calculateDepth(Node *node);

// Main function
int main() {
    int choice;
    parseFile("CS531_Inet.txt");

    while (1) {
        printf("\nMenu:\n");
        printf("1) Add address\n");
        printf("2) Look up address\n");
        printf("3) Update address\n");
        printf("4) Delete address\n");
        printf("5) Display list\n");
        printf("6) Display aliases for location\n");
        printf("7) Display Error Log\n");
        printf("8) Quit\n");
        printf("Enter your choice: ");
        scanf("%d", &choice);

        switch (choice) {
            case 1:
                addAddress();
                break;
            case 2:
                lookUpAddress();
                break;
            case 3:
                updateAddress();
                break;
            case 4:
                deleteAddress();
                break;
            case 5:
                displayList(root);
                break;
            case 6:
                displayAliasesForLocation();
                break;
            case 7:
                displayErrorLog();
                break;
            case 8:
                freeTree(root);
                printf("Exiting program. Goodbye!\n");
                exit(0);
            default:
                printf("Invalid choice. Please try again.\n");
        }
    }

    return 0;
}

// Function to validate IPv4 address format
int validateIP(char *ip) {
    int octets[4];
    char extra; // To catch any leftover characters after the expected format

    // Ensure the input matches exactly four octets and nothing more
    if (sscanf(ip, "%d.%d.%d.%d%c", &octets[0], &octets[1], &octets[2], &octets[3], &extra) != 4)
        return 0;

    // Validate each octet is in the range [0, 255]
    for (int i = 0; i < 4; i++) {
        if (octets[i] < 0 || octets[i] > 255)
            return 0;
    }

    // Ensure the string has exactly three dots
    int dotCount = 0;
    for (int i = 0; ip[i] != '\0'; i++) {
        if (ip[i] == '.')
            dotCount++;
    }
    if (dotCount != 3)
        return 0;

    return 1;
}


// Function to validate alias (length and no uppercase letters)
int validateAlias(char *alias) {
    if (strlen(alias) > MAX_ALIAS_LENGTH)
        return 0;

    return 1;
}

// Check for duplicate IP or alias in the tree
int isDuplicate(Node *node, char *ip, char *alias) {
    if (!node)
        return 0;

    // Check for duplicate IP if provided
    if (ip && strcmp(node->ip, ip) == 0)
        return 1;

    // Check for duplicate alias if provided
    if (alias && strcasecmp(node->alias, alias) == 0) // Use strcasecmp for case-insensitive comparison
        return 1;

    // Recursively check in the left and right subtrees
    return isDuplicate(node->left, ip, alias) || isDuplicate(node->right, ip, alias);
}

// Function to parse the input file
void parseFile(const char *filename) {
    FILE *file = fopen(filename, "r");
    FILE *logFile = fopen("CS531_error-log.txt", "w");
    if (!file) {
        perror("Error opening input file");
        exit(1);
    }

    char line[MAX_LINE_LENGTH];
    while (fgets(line, sizeof(line), file)) {
        char ip[16], alias[2*MAX_ALIAS_LENGTH];

        // Extract the IP and alias from the line
        if (sscanf(line, "%s %s", ip, alias) != 2) {
            fprintf(logFile, "Invalid line format: %s", line);
            continue;
        }

        // Check for IP, alias validation, duplicates, and uppercase letters
        if (!validateIP(ip) || !validateAlias(alias) || containsUppercase(alias)) {
            fprintf(logFile, "Invalid entry: %s", line);
            
        } else {
            root = insert(root, ip, alias);
        }
    }

    fclose(file);
    fclose(logFile);
}

// Function to check uppercase present in alias
int containsUppercase(const char *alias) {
    while (*alias) {
        if (*alias >= 'A' && *alias <= 'Z') {
            return 1; // Uppercase letter found
        }
        alias++;
    }
    return 0; // No uppercase letters
}


// AVL tree insertion
Node *insert(Node *node, char *ip, char *alias) {
    if (node == NULL) {
        Node *newNode = (Node *)malloc(sizeof(Node));
        strcpy(newNode->ip, ip);
        strcpy(newNode->alias, alias);
        newNode->height = 1;
        newNode->left = newNode->right = newNode->parent = NULL;
        return newNode;
    }

    // Recurse down the tree to find the correct insertion point
    if (strcmp(alias, node->alias) < 0) {
        node->left = insert(node->left, ip, alias);
        node->left->parent = node;  // Set parent pointer
    } else if (strcmp(alias, node->alias) > 0) {
        node->right = insert(node->right, ip, alias);
        node->right->parent = node; // Set parent pointer
    }

    root = rebalance(node);
    updateHeight(root);
    return root;
}


// Function to rebalance the AVL tree
Node *rebalance(Node *node) {
    int balance = getBalanceFactor(node);

    if (balance > 1) {
        if (getBalanceFactor(node->left) >= 0)
            return rotateRight(node);
        else
            return rotateLeftThenRight(node);
    } else if (balance < -1) {
        if (getBalanceFactor(node->right) <= 0)
            return rotateLeft(node);
        else
            return rotateRightThenLeft(node);
    }

    return node;
}


// Rotate left
Node *rotateLeft(Node *node) {
    Node *newRoot = node->right;
    node->right = newRoot->left;
    if (newRoot->left != NULL) {
        newRoot->left->parent = node;  // Update parent
    }
    newRoot->left = node;
    newRoot->parent = node->parent;
    node->parent = newRoot;

    // Update the heights
    updateHeight(node);
    updateHeight(newRoot);

    return rebalance(newRoot);
}

Node *rotateRight(Node *node) {
    Node *newRoot = node->left;
    node->left = newRoot->right;
    if (newRoot->right != NULL) {
        newRoot->right->parent = node;  // Update parent
    }
    newRoot->right = node;
    newRoot->parent = node->parent;
    node->parent = newRoot;

    // Update the heights
    updateHeight(node);
    updateHeight(newRoot);

    return rebalance(newRoot);
}


// Rotate left then right
Node *rotateLeftThenRight(Node *node) {
    node->left = rotateLeft(node->left);
    updateHeight(node);
    return rebalance(rotateRight(node));
}

// Rotate right then left
Node *rotateRightThenLeft(Node *node) {
    node->right = rotateRight(node->right);
    updateHeight(node);
    return rebalance(rotateLeft(node));
}

// Get height of a node
int getHeight(Node *node) {
    return node ? node->height : 0;
}

// Get balance factor of a node
int getBalanceFactor(Node *node) {
    if (node == NULL) return 0;
    return getHeight(node->left) - getHeight(node->right);
}

// Function to update height
void updateHeight(Node *node) {
    if (!node) return;
    int leftHeight = (node->left) ? node->left->height : 0;  // Height of left child or 0
    int rightHeight = (node->right) ? node->right->height : 0; // Height of right child or 0

    // Height is the max height of children + 1
    node->height = (leftHeight > rightHeight ? leftHeight : rightHeight)+1;
}

// Function to look up an alias
void lookUpAddress() {
    char alias[MAX_ALIAS_LENGTH + 1];
    printf("Enter alias to look up: ");
    scanf("%s", alias);

    Node *current = root;
    while (current) {
        if (strcmp(alias, current->alias) == 0) {
            printf("IP address for alias '%s': %s\n", alias, current->ip);
            return;
        } else if (strcmp(alias, current->alias) < 0) {
            current = current->left;
        } else {
            current = current->right;
        }
    }
    printf("Alias '%s' not found.\n", alias);
}

// Function to delete a node from the AVL tree
void deleteAddress() {
    char alias[MAX_ALIAS_LENGTH + 1];
    printf("Enter the alias to delete: ");
    scanf("%s", alias);

    Node *target = findNodeByAlias(root, alias);

    if (!target) {
        printf("Alias '%s' not found.\n", alias);
        return;
    }

    char confirm;
    printf("Are you sure you want to delete alias '%s'? (y/n): ", alias);
    scanf(" %c", &confirm);
    if (confirm != 'y') {
        printf("Deletion canceled.\n");
        return;
    }

    // Perform deletion
    root = deleteNode(root, alias);
    printf("Alias '%s' deleted successfully.\n", alias);
}

// Helper function to delete a node and maintain AVL balance
Node *deleteNode(Node *node, const char *alias) {
    if (!node) return NULL;

    if (strcmp(alias, node->alias) < 0) {
        node->left = deleteNode(node->left, alias);
    } else if (strcmp(alias, node->alias) > 0) {
        node->right = deleteNode(node->right, alias);
    } else {
        // Node with only one child or no child
        if (!node->left || !node->right) {
            Node *temp = node->left ? node->left : node->right;

            if (!temp) {
                // No child case
                temp = node;
                node = NULL;
            } else {
                // One child case
                *node = *temp; // Copy the content of the child to the current node
            }

            free(temp);
        } else {
            // Node with two children: Get the in-order successor
            Node *temp = node->right;
            while (temp->left) {
                temp = temp->left;
            }

            // Copy successor's data to current node
            strcpy(node->ip, temp->ip);
            strcpy(node->alias, temp->alias);

            // Delete the in-order successor
            node->right = deleteNode(node->right, temp->alias);
        }
    }

    if (!node) return NULL;

    // Update height and rebalance the node
    updateHeight(node);
    return rebalance(node);
}


// Function to display the AVL tree nodes
void displayList(Node *node) {
    if (!node)
        return;

    displayList(node->left);

    int depth = calculateDepth(node); // Calculate depth of the node
    printf("Alias: %s, IP: %s, Height: %d, Depth: %d, Balance Factor: %d, ",
           node->alias, node->ip, node->height-1, depth, getBalanceFactor(node));

    if (node->parent)
        printf("Parent: %s\n", node->parent->alias);
    else
        printf("Parent: None (Root Node)\n");

    displayList(node->right);
}

// Function to display error log
void displayErrorLog() {
    FILE *logFile = fopen("CS531_error-log.txt", "r");
    if (!logFile) {
        perror("Error opening log file");
        return;
    }

    printf("Error Log:\n");
    char line[MAX_LINE_LENGTH];
    while (fgets(line, sizeof(line), logFile)) {
        printf("%s", line);
    }

    fclose(logFile);
}

// Function to free the AVL tree
void freeTree(Node *node) {
    if (!node)
        return;

    freeTree(node->left);
    freeTree(node->right);
    free(node);
}

//Funtion to add IP address
#include <ctype.h> // For tolower()

void addAddress() {
    char ip[20], alias[2*MAX_ALIAS_LENGTH];
    while (1) {
        
        printf("Enter IP address: ");
        scanf("%s", ip);

        if (!validateIP(ip)) {
            printf("Invalid IP address format. Please re-enter.\n");
            continue;
        }

        if (isDuplicate(root, ip, NULL)) {
            printf("Duplicate IP address. Please enter a unique IP.\n");
            continue;
        }

        break;
    }

    while (1) {
        printf("Enter alias (max 10 characters, no uppercase): ");
        scanf("%s", alias);

        if (strlen(alias) > MAX_ALIAS_LENGTH) {
            printf("Invalid alias. Please ensure it's at most 10 characters long.\n");
            continue;
        }

        if (isDuplicate(root, NULL, alias)) {
            printf("Duplicate alias. Please enter a unique alias.\n");
            continue;
        }

        break;
    }

    // Convert alias to lowercase
    for (int i = 0; alias[i]; i++) {
        alias[i] = tolower(alias[i]);
    }

    root = insert(root, ip, alias);
    printf("Address added successfully.\n");
}


void updateAddress() {
    char alias[MAX_ALIAS_LENGTH + 1];
    printf("Enter alias to update: ");
    scanf("%s", alias);

    Node *node = findNodeByAlias(root, alias);
    if (!node) {
        printf("Alias '%s' not found.\n", alias);
        return;
    }

    char newIP[20];
    while (1) {
        printf("Enter new IP address: ");
        scanf("%s", newIP);

        if (!validateIP(newIP)) {
            printf("Invalid IP address format. Please re-enter.\n");
            continue;
        }

        if (isDuplicate(root, newIP, NULL)) {
            printf("Duplicate IP address. Please enter a unique IP.\n");
            continue;
        }

        break;
    }

    strcpy(node->ip, newIP);

    // Rebalance the tree starting from the updated node's parent
    Node *parent = node->parent;
    while (parent) {
        parent = rebalance(parent)->parent;
    }
    root = rebalance(root);

    printf("IP address updated successfully.\n");
}

// Function to find node with the help of given 2 octets of an IP
void displayAliasesForLocation() {
    char prefix[8];
    printf("Enter the first two octets of an IPv4 address (e.g., '192.168'): ");
    scanf("%s", prefix);

    if (!isValidIPv4Prefix(prefix)) {
        printf("Invalid prefix format. Ensure it contains only the first two octets.\n");
        return;
    }

    printf("Aliases for IPs starting with '%s':\n", prefix);
    int found = 0;
    displayAliasesForLocationHelper(root, prefix, &found);
    if (!found) {
        printf("No matching aliases found.\n");
    }
}

//Helper FUnction
Node *findNodeByIP(Node *node, const char *ip) {
    if (!node)
        return NULL;

    if (strcmp(node->ip, ip) == 0)
        return node;

    if (strcmp(ip, node->ip) < 0)
        return findNodeByIP(node->left, ip);
    else
        return findNodeByIP(node->right, ip);
}

//Helper Function
Node *findNodeByAlias(Node *node, const char *alias) {
    if (!node)
        return NULL;

    if (strcmp(node->alias, alias) == 0)
        return node;

    if (strcmp(alias, node->alias) < 0)
        return findNodeByAlias(node->left, alias);
    else
        return findNodeByAlias(node->right, alias);
}

//Helper Function
int isValidIPv4Prefix(const char *prefix) {
    char temp[16];
    snprintf(temp, sizeof(temp), "%s.0.0", prefix); // Append placeholders for full IP validation
    return validateIP(temp);
}

//HElper function
void displayAliasesForLocationHelper(Node *node, const char *prefix, int *found) {
    if (!node)
        return;

    if (strncmp(node->ip, prefix, strlen(prefix)) == 0) {
        printf("Alias: %s, IP: %s\n", node->alias, node->ip);
        *found = 1;
    }

    displayAliasesForLocationHelper(node->left, prefix, found);
    displayAliasesForLocationHelper(node->right, prefix, found);
}

// Function to calculate the depth of the node
int calculateDepth(Node *node) {
    int depth = 0;
    while (node && node->parent) {
        node = node->parent;
        depth++;
    }
    return depth;
}