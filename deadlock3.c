#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define MAX_RESOURCES 100
#define MAX_PROCESSES 100

// Forward declaration of Resource structure
typedef struct Resource Resource;

// Structure to represent a process
typedef struct {
    int id;
    Resource* holding;
    Resource* waiting;
} Process;

// Structure to represent a resource
struct Resource {
    int id;
    int site;
    int heldBy; // Process ID of the process holding this resource, -1 if not held
};

// Function to check for cycles in the resource allocation graph
bool detectCycle(Process* processes, Resource* resources, int numProcesses, Process* cur, int start) {
    if (cur->holding == NULL) {
        return false; // If the process is not holding any resource, it cannot be part of a cycle
    }

    for (int i = 0; i < numProcesses; i++) {
        if (cur->waiting != NULL && processes[i].holding != NULL && cur->waiting->id == processes[i].holding->id) {
            if (processes[i].id == start) {
                return true;
            } else {
                if (detectCycle(processes, resources, numProcesses, &processes[i], start)) {
                    return true;
                }
            }
        }
    }
    return false;
}


// Function to check for deadlock in a site
bool checkDeadlockSite(Process* processes, Resource* resources, int site, int numProcesses) {
    for (int i = 0; i < numProcesses; i++) {
        if (processes[i].id != -1 && processes[i].holding != NULL && processes[i].waiting != NULL &&
            processes[i].holding->site == site && processes[i].waiting->site == site) {
            if (detectCycle(processes, resources, numProcesses, &processes[i], processes[i].id)) {
                return true;
            }
        }
    }
    return false;
}

// Function to check for deadlock in the coordinator
bool checkDeadlock(Process* processes, Resource* resources, int numProcesses) {
    for (int i = 0; i < numProcesses; i++) {
        if (processes[i].waiting != NULL && detectCycle(processes, resources, numProcesses, &processes[i], processes[i].id)) {
            // Check if the waiting resource is from a different site
            bool waitingFromDifferentSite = false;
            for (int j = 0; j < numProcesses; j++) {
                if (processes[j].id != -1 && processes[j].holding != NULL && processes[j].waiting != NULL) {
                    if (processes[j].holding->site != processes[j].waiting->site) {
                        waitingFromDifferentSite = true;
                        break;
                    }
                }
            }
            if (waitingFromDifferentSite) {
                return true; // Global deadlock detected
            } else {
                return false; // Deadlock within a site, not global
            }
        }
    }
    return false;
}

int main() {
    Resource resources[MAX_RESOURCES];
    Process processes[MAX_PROCESSES];

    // Initialize processes
    for (int i = 0; i < MAX_PROCESSES; i++) {
        processes[i].id = -1; // Indicates an empty slot
        processes[i].holding = NULL;
        processes[i].waiting = NULL;
    }

    // Input the number of sites and resources
    int numSites, numResources;
    printf("Enter the number of sites: ");
    if (scanf("%d", &numSites) != 1 || numSites <= 0 || numSites > MAX_RESOURCES) {
        printf("Invalid input for the number of sites.\n");
        return 1;
    }

    printf("Enter the number of resources: ");
    if (scanf("%d", &numResources) != 1 || numResources <= 0 || numResources > MAX_RESOURCES) {
        printf("Invalid input for the number of resources.\n");
        return 1;
    }

    int resourceIdCounter = 0;

    // Initialize resources for each site
    for (int s = 1; s <= numSites; s++) {
        int numSiteResources;
        printf("Enter the number of resources in site %d: ", s);
        if (scanf("%d", &numSiteResources) != 1 || numSiteResources < 0 || numSiteResources > numResources) {
            printf("Invalid input for the number of resources in site %d.\n", s);
            return 1;
        }

        for (int i = 0; i < numSiteResources; i++) {
            resources[resourceIdCounter].id = resourceIdCounter;
            resources[resourceIdCounter].site = s;
            resources[resourceIdCounter].heldBy = -1; // Initially not held by any process
            resourceIdCounter++;
        }
    }

    // Display resources in each site
    for (int s = 1; s <= numSites; s++) {
        printf("\nResources in site %d:\n", s);
        for (int i = 0; i < numResources; i++) {
            if (resources[i].site == s) {
                printf("%d ", resources[i].id);
            }
        }
        printf("\n");
    }
    printf("\n");

    // Input processes
    int numProcesses;
    printf("Enter the number of processes: ");
    if (scanf("%d", &numProcesses) != 1 || numProcesses <= 0 || numProcesses > MAX_PROCESSES) {
        printf("Invalid input for the number of processes.\n");
        return 1;
    }

    for (int i = 0; i < numProcesses; i++) {
        int hld, wai, site;
        printf("Enter the site for process-%d: ", i);
        if (scanf("%d", &site) != 1 || site <= 0 || site > numSites) {
            printf("Invalid input for the site of process-%d.\n", i);
            return 1;
        }

        printf("What resource is process-%d holding? (Enter -1 for none): ", i);
        if (scanf("%d", &hld) != 1 || hld < -1 || hld >= numResources) {
            printf("Invalid input for the holding resource of process-%d.\n", i);
            return 1;
        }

        printf("What resource is process-%d waiting for? (Enter -1 for none): ", i);
        if (scanf("%d", &wai) != 1 || wai < -1 || wai >= numResources) {
            printf("Invalid input for the waiting resource of process-%d.\n", i);
            return 1;
        }

        processes[i].id = i;
        processes[i].holding = (hld != -1) ? &resources[hld] : NULL;
        processes[i].waiting = (wai != -1) ? &resources[wai] : NULL;
    }

    bool globalDeadlock = checkDeadlock(processes, resources, numProcesses);

    for (int s = 1; s <= numSites; s++) {
        bool siteDeadlock = checkDeadlockSite(processes, resources, s, numProcesses);
        if (siteDeadlock) {
            printf("Deadlock detected in site %d\n", s);
        }
    }

    if (globalDeadlock) {
        printf("Deadlock detected in the central coordinator\n");
    }

    if (!globalDeadlock) {
        printf("No deadlock detected\n");
    }

    return 0;
}

