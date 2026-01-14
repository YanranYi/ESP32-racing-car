#ifndef NAVIGATION_H
#define NAVIGATION_H

#include "field_config.h"

// ============ Dual Vive Configuration ============
#ifndef VIVE_OFFSET_X
#define VIVE_OFFSET_X 5
#endif
#ifndef VIVE_OFFSET_Y  
#define VIVE_OFFSET_Y 5
#endif

// If the vehicle keeps rotating in place, it might be because sensor A/B is installed in the wrong position. Please switch it to "true"
#define VIVE_SENSORS_REVERSED false

// ============ Vive Reliability Settings ============
#define VIVE_QUALITY_MAX 10

// ============ velocity parameters ============
#define navSpeedRamp 255      // Going uphill at full speed
#define navTurnSpeed 160      // Rotation speed in place

// ============ TOF obstacle avoidance paraters ============
#define TOF_NAV_ENABLED false
#define TOF_NAV_OBSTACLE_DIST 120
#define TOF_NAV_PANIC_DIST 50
#define TOF_NAV_TURN_AMOUNT 25
#define TOF_NAV_ESCAPE_TIME 300

// ============ Low Tower Fixed-point obstacle avoidance ============
#define LOW_TOWER_Y_MIN 3850
#define LOW_TOWER_Y_MAX 4370

// Two fixed orbiting points
#define BYPASS_LEFT_X  4090
#define BYPASS_LEFT_Y  4107
#define BYPASS_RIGHT_X 5100
#define BYPASS_RIGHT_Y 4200

// ============ ramp area ============
#define RAMP_AVOID_X_MIN (RAMP_X_MIN - 200)
#define RAMP_AVOID_X_MAX (RAMP_X_MAX + 100)
#define RAMP_AVOID_Y_MIN (RAMP_ENTRY_Y - 100)
#define RAMP_AVOID_Y_MAX (RAMP_EXIT_Y + 100)

// ============ Path Planning Parameters ============
#define MAX_WAYPOINTS 10
#define LOW_TOWER_AVOID_MARGIN 600
#define ARRIVAL_TOLERANCE 120
#define ARRIVAL_CONFIRM_COUNT
// ============ Steering control parameters ============
#define HEADING_DEADZONE 0.17
#define HEADING_SMALL_ERROR 0.52
#define HEADING_MED_ERROR 1.05
#define HEADING_KP_SMALL 12.0
#define HEADING_KP_MED 18.0
#define HEADING_KP_LARGE 22.0
#define HEADING_SMOOTH_ALPHA 0.15

// ============ velocity parameters ============
#define SPEED_FAST 60
#define SPEED_NORMAL 50
#define SPEED_SLOW 40
#define SPEED_APPROACH 35
#define SPEED_MIN 30

// ============ High Tower time parameters ============
#define HT_TIME_APPROACH  7200
#define HT_TIME_TURN      700
#define HT_TIME_HIT       16000

// ============ Low Tower time parameters ============
#define LT_TIME_FWD_1   7500
#define LT_TIME_TURN_1  1000
#define LT_TIME_FWD_2   1100
#define LT_TIME_TURN_2  800
#define LT_TIME_FWD_3   16000

// ============ Nexus time parameters ============
#define NX_TIME_FWD_1     11000
#define NX_TIME_TURN_1    1100
#define NX_TIME_FWD_2     3000
#define NX_HIT_FWD_TIME   600
#define NX_HIT_BCK_TIME   400
#define NX_HIT_COUNT      4

// ============ Navigation States ============
enum NavState {
    NAV_IDLE,
    NAV_MANUAL,
    NAV_WALL_FOLLOW,
    NAV_TO_WAYPOINT,
    NAV_TO_RAMP_ENTRY,
    NAV_CLIMBING_RAMP,
    NAV_ON_PLATFORM,
    NAV_TURN_TO_BUTTON,
    NAV_APPROACH_TARGET,
    NAV_ATTACKING,
    NAV_RETREAT,
    NAV_LOW_TOWER_APPROACH,
    NAV_NEXUS_ATTACKING,
    NAV_NEXUS_RETREATING,
    
    // High Tower
    NAV_HT_BLIND_FORWARD,
    NAV_HT_BLIND_TURN,
    NAV_HT_BLIND_HIT,
    NAV_HT_BLIND_FORWARD2,

    // Low Tower
    NAV_LT_BLIND_FWD_1,
    NAV_LT_BLIND_TURN_1,
    NAV_LT_BLIND_FWD_2,
    NAV_LT_BLIND_TURN_2,
    NAV_LT_BLIND_FWD_3,

    // Nexus
    NAV_NX_BLIND_FWD_1,
    NAV_NX_BLIND_TURN_1,
    NAV_NX_BLIND_FWD_2,
    NAV_NX_BLIND_HIT_FWD,
    NAV_NX_BLIND_HIT_BCK,
    
    // smart navigation state
    NAV_AVOIDING_OBSTACLE,   // Avoiding obstacles in the middle
    NAV_ENTERING_RAMP,       // enter the ramp
    NAV_ON_RAMP,             // Driving on the slope
    
    NAV_TOF_AVOIDING,
    NAV_ERROR
};

// ============ Task Types ============
enum NavTask {
    TASK_NONE,
    TASK_GOTO_SINGLE,
    TASK_GOTO_THREE,
    TASK_GOTO_SMART,        // smart navigation task
    TASK_CAPTURE_LOW,
    TASK_CAPTURE_HIGH,
    TASK_ATTACK_NEXUS,
    TASK_WALL_FOLLOW_FULL,
    TASK_COVER_VIVE_LOCS
};

// ============ Single Vive Sensor Data ============
struct ViveSensorData {
    int x;
    int y;
    bool valid;
    int quality;
    unsigned long lastUpdate;
};

// ============ Combined Vive Data ============
struct ViveData {
    int x;
    int y;
    float heading;
    bool valid;
    int quality;
    bool reliable;
    unsigned long lastUpdate;
    
    ViveSensorData sensorA;
    ViveSensorData sensorB;
};

// ============ Navigation Target ============
struct NavTarget {
    int x;
    int y;
    int tolerance;
    bool useVive;
    int approachSpeed;
    bool isRampWaypoint;   // whether it is a ramp landing point
    bool isFinalTarget;    // whether it is the final target
};

struct TofAvoidState {
    bool active;
    int turnDirection;
    unsigned long startTime;
    NavState returnState;
};

// ============ Global Navigation Variables ============
int navSpeedFast = AUTO_SPEED_FAST;
int navSpeedNormal = AUTO_SPEED_NORMAL;
int navSpeedSlow = AUTO_SPEED_SLOW;
int navSpeedApproach = AUTO_SPEED_APPROACH;
// int navSpeedRamp = AUTO_SPEED_RAMP_UP;
//int navTurnSpeed = 40;

NavState navState = NAV_IDLE;
NavTask currentTask = TASK_NONE;
int currentTeam = TEAM_BLUE;

ViveData vive;
TofAvoidState tofAvoid = {false, 0, 0, NAV_IDLE};

NavTarget currentTarget;
NavTarget waypoints[MAX_WAYPOINTS];
int waypointCount = 0;
int currentWaypointIndex = 0;

int finalTargetX = 0;
int finalTargetY = 0;

unsigned long navStartTime = 0;
unsigned long taskStartTime = 0;
int nexusAttackCount = 0;

int navSpeedCmd = 0;
int navTurnCmd = 0;
int navDirCmd = 1;

float debugNavDistance = 0;
float debugNavHeading = 0;
float debugNavHeadingError = 0;
float debugViveHeading = 0;

// arrive counter
static int arrivalCounter = 0;

// Heading smootherness
static float smoothedHeading = 0;
static bool headingInitialized = false;

// external TOF data
extern int tofFrontDist;
extern int tofLeft1Dist;
extern int tofLeft2Dist;

// ============ Function Declarations ============
void navInit();
void navUpdate();
void navStop();
void updateDualViveFromRaw(int xA, int yA, bool validA, int xB, int yB, bool validB);
void calculateNavCommands();
void taskGotoSingle(int x, int y);
void taskGotoSmart(int x, int y);
void taskGotoThree(int x1, int y1, int x2, int y2, int x3, int y3);
void taskCaptureLowTower();
void taskCaptureHighTower();
void taskAttackNexus();

// ============ Initialize Navigation System ============
void navInit() {
    navState = NAV_IDLE;
    currentTask = TASK_NONE;
    
    vive.sensorA.valid = false;
    vive.sensorA.quality = 0;
    vive.sensorB.valid = false;
    vive.sensorB.quality = 0;
    
    vive.valid = false;
    vive.quality = 0;
    vive.heading = 0;
    
    tofAvoid.active = false;
    arrivalCounter = 0;
    headingInitialized = false;
    
    navSpeedCmd = 0;
    navTurnCmd = 0;
    nexusAttackCount = 0;
    waypointCount = 0;
    currentWaypointIndex = 0;
}

// ============ Vive update ============
void updateDualViveFromRaw(int xA, int yA, bool validA, int xB, int yB, bool validB) {
    unsigned long now = millis();
    
    // update sensor A
    if (validA && xA > 0 && yA > 0) {
        vive.sensorA.x = xA;
        vive.sensorA.y = yA;
        vive.sensorA.valid = true;
        vive.sensorA.quality = VIVE_QUALITY_MAX;
        vive.sensorA.lastUpdate = now;
    } else if (now - vive.sensorA.lastUpdate > 500) {
        vive.sensorA.valid = false;
        vive.sensorA.quality = 0;
    }
    
    // update sensor B
    if (validB && xB > 0 && yB > 0) {
        vive.sensorB.x = xB;
        vive.sensorB.y = yB;
        vive.sensorB.valid = true;
        vive.sensorB.quality = VIVE_QUALITY_MAX;
        vive.sensorB.lastUpdate = now;
    } else if (now - vive.sensorB.lastUpdate > 500) {
        vive.sensorB.valid = false;
        vive.sensorB.quality = 0;
    }
    
    // Calculate the fusion position and heading
    if (vive.sensorA.valid && vive.sensorB.valid) {
        vive.x = (vive.sensorA.x + vive.sensorB.x) / 2;
        vive.y = (vive.sensorA.y + vive.sensorB.y) / 2;
        
        int dx, dy;
        if (VIVE_SENSORS_REVERSED) {
            dx = vive.sensorA.x - vive.sensorB.x;
            dy = vive.sensorA.y - vive.sensorB.y;
        } else {
            dx = vive.sensorB.x - vive.sensorA.x;
            dy = vive.sensorB.y - vive.sensorA.y;
        }
        
        vive.heading = atan2(dx, dy);
        debugViveHeading = vive.heading * 180.0 / PI;
        
        vive.valid = true;
        vive.quality = VIVE_QUALITY_MAX;
        vive.lastUpdate = now;
        
    } else if (vive.sensorA.valid) {
        vive.x = vive.sensorA.x;
        vive.y = vive.sensorA.y;
        vive.valid = true;
        vive.quality = vive.sensorA.quality;
        vive.lastUpdate = now;
        
    } else if (vive.sensorB.valid) {
        vive.x = vive.sensorB.x;
        vive.y = vive.sensorB.y;
        vive.valid = true;
        vive.quality = vive.sensorB.quality;
        vive.lastUpdate = now;
        
    } else {
        vive.valid = false;
        vive.quality = 0;
    }
}

// ============ Check if Vive is Valid ============
bool isViveValid() {
    return vive.valid;
}

// ============ Path obstacle detection ============
bool isPathCrossingRamp(int fromX, int fromY, int toX, int toY) {
    int steps = 20;
    for (int i = 1; i < steps; i++) {
        float t = (float)i / steps;
        int checkX = fromX + (int)((toX - fromX) * t);
        int checkY = fromY + (int)((toY - fromY) * t);
        
        if (checkX >= RAMP_AVOID_X_MIN && checkX <= RAMP_AVOID_X_MAX &&
            checkY >= RAMP_AVOID_Y_MIN && checkY <= RAMP_AVOID_Y_MAX) {
            return true;
        }
    }
    return false;
}

bool isPathCrossingLowTower(int fromX, int fromY, int toX, int toY) {
    int steps = 20;
    for (int i = 1; i < steps; i++) {
        float t = (float)i / steps;
        int checkX = fromX + (int)((toX - fromX) * t);
        int checkY = fromY + (int)((toY - fromY) * t);
        
        if (isInLowTowerPoly(checkX, checkY)) {
            return true;
        }
    }
    return false;
}

bool isPathCrossingAnyObstacle(int fromX, int fromY, int toX, int toY) {
    return isPathCrossingLowTower(fromX, fromY, toX, toY) || 
           isPathCrossingRamp(fromX, fromY, toX, toY);
}

// ============ Determine whether it is necessary to bypass Low Tower ============
bool needsBypassLowTower(int fromY, int toY) {
    if (fromY < LOW_TOWER_Y_MIN && toY > LOW_TOWER_Y_MAX) return true;
    if (fromY > LOW_TOWER_Y_MAX && toY < LOW_TOWER_Y_MIN) return true;
    return false;
}

// ============ Select the detour point ============
void getBypassPoint(int fromX, int& bypassX, int& bypassY) {
    if (fromX < 4400) {
        bypassX = BYPASS_RIGHT_X;
        bypassY = BYPASS_RIGHT_Y;
    } else {
        bypassX = BYPASS_LEFT_X;
        bypassY = BYPASS_LEFT_Y;
    }
}

// ============ auxiliary function ============
static inline void clampToSafeZone(int &x, int &y) {
    x = constrain(x, VIVE_SAFE_X_MIN, VIVE_SAFE_X_MAX);
    y = constrain(y, VIVE_SAFE_Y_MIN, VIVE_SAFE_Y_MAX);
}

static inline void pushWaypoint(int x, int y, bool isFinal, int speed) {
    if (waypointCount >= MAX_WAYPOINTS) return;
    waypoints[waypointCount].x = x;
    waypoints[waypointCount].y = y;
    waypoints[waypointCount].tolerance = ARRIVAL_TOLERANCE;
    waypoints[waypointCount].approachSpeed = speed;
    waypoints[waypointCount].useVive = true;
    waypoints[waypointCount].isRampWaypoint = false;
    waypoints[waypointCount].isFinalTarget = isFinal;
    waypointCount++;
}

// ============ Intelligent path planning ============
int findSmartPath(int startX, int startY, int goalX, int goalY) {
    waypointCount = 0;
    
    // strategy 1:Try the vertical-horizontal path
    if (!isPathCrossingAnyObstacle(startX, startY, startX, goalY) &&
        !isPathCrossingAnyObstacle(startX, goalY, goalX, goalY)) {
        pushWaypoint(startX, goalY, false, SPEED_NORMAL);
        pushWaypoint(goalX, goalY, true, SPEED_NORMAL);
        return 1;
    }
    
    // strategy 2:Try the horizontal-vertical path
    if (!isPathCrossingAnyObstacle(startX, startY, goalX, startY) &&
        !isPathCrossingAnyObstacle(goalX, startY, goalX, goalY)) {
        pushWaypoint(goalX, startY, false, SPEED_NORMAL);
        pushWaypoint(goalX, goalY, true, SPEED_NORMAL);
        return 1;
    }
    
    // strategy 3:Fixed-point detour
    if (needsBypassLowTower(startY, goalY)) {
        int bypassX, bypassY;
        getBypassPoint(startX, bypassX, bypassY);
        
        if (!isPathCrossingAnyObstacle(startX, startY, bypassX, bypassY) &&
            !isPathCrossingAnyObstacle(bypassX, bypassY, goalX, goalY)) {
            pushWaypoint(bypassX, bypassY, false, SPEED_NORMAL);
            pushWaypoint(goalX, goalY, true, SPEED_NORMAL);
            return 1;
        }
    }
    
    return 0;
}

// ============ Corridor detour (backup plan) ============
void planCorridorPath(int fromX, int fromY, int toX, int toY) {
    waypointCount = 0;
    
    int corridorOptions[] = {
        VIVE_SAFE_Y_MIN + 500,
        VIVE_SAFE_Y_MAX - 500,
        (VIVE_SAFE_Y_MIN + VIVE_SAFE_Y_MAX) / 2
    };
    
    for (int i = 0; i < 3; i++) {
        int corridorY = corridorOptions[i];
        
        if (!isPathCrossingAnyObstacle(fromX, fromY, fromX, corridorY) &&
            !isPathCrossingAnyObstacle(fromX, corridorY, toX, corridorY) &&
            !isPathCrossingAnyObstacle(toX, corridorY, toX, toY)) {
            
            if (abs(fromY - corridorY) > 100) {
                pushWaypoint(fromX, corridorY, false, SPEED_NORMAL);
            }
            pushWaypoint(toX, corridorY, false, SPEED_NORMAL);
            pushWaypoint(toX, toY, true, SPEED_NORMAL);
            return;
        }
    }
    
    // All strategies failed, just go directly
    pushWaypoint(toX, toY, true, SPEED_SLOW);
}

// ============ Heading smoothing filtering ============
float smoothHeading(float newHeading) {
    if (!headingInitialized) {
        smoothedHeading = newHeading;
        headingInitialized = true;
        return smoothedHeading;
    }
    
    float diff = newHeading - smoothedHeading;
    while (diff > PI) diff -= 2*PI;
    while (diff < -PI) diff += 2*PI;
    
    smoothedHeading += HEADING_SMOOTH_ALPHA * diff;
    
    while (smoothedHeading > PI) smoothedHeading -= 2*PI;
    while (smoothedHeading < -PI) smoothedHeading += 2*PI;
    
    return smoothedHeading;
}

// ============ Calculate Navigation Commands ============
void calculateNavCommands() {
    if (!isViveValid()) {
        navSpeedCmd = SPEED_MIN;
        navTurnCmd = 0;
        return;
    }
    
    float dx = currentTarget.x - vive.x;
    float dy = currentTarget.y - vive.y;
    debugNavDistance = sqrt(dx*dx + dy*dy);
    
    static unsigned long lastDebug = 0;
    if (millis() - lastDebug > 500) {
        lastDebug = millis();
    }
    
    // arrival detection
    if (debugNavDistance < currentTarget.tolerance) {
        navSpeedCmd = 0;
        navTurnCmd = 0;
        return;
    }
    
    // Calculate the target course
    float targetHeading = atan2(dx, dy);
    debugNavHeading = targetHeading * 180.0 / PI;
    
    // Use the smoothed heading
    float currentHeading = smoothHeading(vive.heading);
    
    // Heading error
    float headingError = targetHeading - currentHeading;
    while (headingError > PI) headingError -= 2*PI;
    while (headingError < -PI) headingError += 2*PI;
    debugNavHeadingError = headingError * 180.0 / PI;
    
    // ========== Steering control (segmented gain + dead zone) ==========
    float absError = abs(headingError);
    int turnAmount = 0;
    
    if (absError < HEADING_DEADZONE) {
        turnAmount = 0;  // dead zone
    } else if (absError < HEADING_SMALL_ERROR) {
        turnAmount = (int)(headingError * HEADING_KP_SMALL);
    } else if (absError < HEADING_MED_ERROR) {
        turnAmount = (int)(headingError * HEADING_KP_MED);
    } else {
        turnAmount = (int)(headingError * HEADING_KP_LARGE);
    }
    
    turnAmount = constrain(turnAmount, -35, 35);
    
    // Reduction of steering when using a single sensor
    if (!vive.sensorA.valid || !vive.sensorB.valid) {
        turnAmount = turnAmount / 2;
    }
    
    // ========== Speed control (always moving forward) ==========
    int speed;
    
    if (debugNavDistance < 150) {
        speed = SPEED_APPROACH;
    } else if (debugNavDistance < 300) {
        speed = SPEED_SLOW;
    } else if (debugNavDistance < 600) {
        speed = SPEED_NORMAL;
    } else {
        speed = SPEED_FAST;
    }
    
    // When the course error is large, the speed will decrease, but it will not fall below the minimum speed
    if (absError > HEADING_MED_ERROR) {
        speed = max(SPEED_MIN, speed - 20);
    } else if (absError > HEADING_SMALL_ERROR) {
        speed = max(SPEED_MIN, speed - 10);
    }
    
    navSpeedCmd = speed;
    navTurnCmd = turnAmount;
    navDirCmd = 1;
}

// ============ TOF obstacle avoidance ============
bool checkTofObstacle() {
    if (!TOF_NAV_ENABLED) return false;
    return (tofFrontDist > 0 && tofFrontDist < TOF_NAV_OBSTACLE_DIST);
}

void handleTofAvoidance() {
    if (!TOF_NAV_ENABLED) return;
    
    if (!tofAvoid.active) {
        if (checkTofObstacle()) {
            tofAvoid.active = true;
            tofAvoid.startTime = millis();
            tofAvoid.returnState = navState;
            
            if (tofLeft1Dist > 0 && tofLeft1Dist < 300) {
                tofAvoid.turnDirection = 1;  // turn right
            } else {
                tofAvoid.turnDirection = -1; // turn left
            }
            
            navState = NAV_TOF_AVOIDING;
        }
    } else {
        if (millis() - tofAvoid.startTime > TOF_NAV_ESCAPE_TIME) {
            if (!checkTofObstacle()) {
                tofAvoid.active = false;
                navState = tofAvoid.returnState;
            } else {
                tofAvoid.turnDirection = -tofAvoid.turnDirection;
                tofAvoid.startTime = millis();
            }
        }
    }
}

// ============ Navigation Update ============
void navUpdate() {
    // TOF obstacle avoidanve check
    if (navState == NAV_TO_WAYPOINT) {
        handleTofAvoidance();
    }
    
    switch (navState) {
        case NAV_IDLE:
            navSpeedCmd = 0;
            navTurnCmd = 0;
            break;
            
        case NAV_TOF_AVOIDING:
            navSpeedCmd = SPEED_SLOW;
            navTurnCmd = tofAvoid.turnDirection * TOF_NAV_TURN_AMOUNT;
            navDirCmd = 1;
            
            if (tofFrontDist > 0 && tofFrontDist < TOF_NAV_PANIC_DIST) {
                navSpeedCmd = SPEED_SLOW;
                navDirCmd = -1;
                navTurnCmd = 0;
            }
            break;
            
        case NAV_TO_WAYPOINT:
            calculateNavCommands();
            
            // Stable arrival detection
            if (isViveValid() && debugNavDistance < currentTarget.tolerance) {
                arrivalCounter++;
                
                if (arrivalCounter >= ARRIVAL_CONFIRM_COUNT) {
                    arrivalCounter = 0;
                    currentWaypointIndex++;
                    
                    if (currentWaypointIndex >= waypointCount) {
                        navState = NAV_IDLE;
                        currentTask = TASK_NONE;
                        navSpeedCmd = 0;
                        navTurnCmd = 0;
                    } else {
                        currentTarget = waypoints[currentWaypointIndex];
                        navStartTime = millis();
                    }
                }
            } else {
                arrivalCounter = 0;
            }
            break;

        // ============ High Tower ============
        case NAV_HT_BLIND_FORWARD:
            navSpeedCmd = navSpeedNormal; navTurnCmd = 0; navDirCmd = 1;
            if (millis() - navStartTime > HT_TIME_APPROACH) {
                navState = NAV_HT_BLIND_TURN; navStartTime = millis();
            }
            break;
        case NAV_HT_BLIND_TURN:
            navSpeedCmd = 0; 
            navTurnCmd = (currentTeam == TEAM_RED) ? -40 : 40;
            navDirCmd = 1;
            if (millis() - navStartTime > HT_TIME_TURN) {
                navState = NAV_HT_BLIND_HIT; navStartTime = millis();
            }
            break;
        case NAV_HT_BLIND_HIT:
            // Increase the speed a little to push the tower
            navSpeedCmd = navSpeedNormal + 10; 
            navTurnCmd = 0; 
            navDirCmd = 1;
            
            if (millis() - navStartTime > HT_TIME_HIT) {
                navState = NAV_RETREAT;
                navStartTime = millis();
            }
            break;

        // ============ Low Tower ============
        case NAV_LT_BLIND_FWD_1:
            navSpeedCmd = navSpeedNormal; navTurnCmd = 0; navDirCmd = 1;
            if (millis() - navStartTime > LT_TIME_FWD_1) {
                navState = NAV_LT_BLIND_TURN_1; navStartTime = millis();
            }
            break;
        case NAV_LT_BLIND_TURN_1:
            navSpeedCmd = 0; navTurnCmd = -40; navDirCmd = 1;
            if (millis() - navStartTime > LT_TIME_TURN_1) {
                navState = NAV_LT_BLIND_FWD_2; navStartTime = millis();
            }
            break;
        case NAV_LT_BLIND_FWD_2:
            navSpeedCmd = navSpeedNormal; navTurnCmd = 0; navDirCmd = 1;
            if (millis() - navStartTime > LT_TIME_FWD_2) {
                navState = NAV_LT_BLIND_TURN_2; navStartTime = millis();
            }
            break;
        case NAV_LT_BLIND_TURN_2:
            navSpeedCmd = 0; navTurnCmd = -40; navDirCmd = 1;
            if (millis() - navStartTime > LT_TIME_TURN_2) {
                navState = NAV_LT_BLIND_FWD_3; navStartTime = millis();
            }
            break;
        case NAV_LT_BLIND_FWD_3:
            navSpeedCmd = navSpeedNormal; navTurnCmd = 0; navDirCmd = 1;
            if (millis() - navStartTime > LT_TIME_FWD_3) {
                navState = NAV_RETREAT;
            }
            break;

        // ============ Nexus ============
        case NAV_NX_BLIND_FWD_1:
            navSpeedCmd = navSpeedNormal; navTurnCmd = 0; navDirCmd = 1;
            if (millis() - navStartTime > NX_TIME_FWD_1) {
                navState = NAV_NX_BLIND_TURN_1; navStartTime = millis();
            }
            break;
        case NAV_NX_BLIND_TURN_1:
            navSpeedCmd = 0; navTurnCmd = -30; navDirCmd = 1;
            if (millis() - navStartTime > NX_TIME_TURN_1) {
                navState = NAV_NX_BLIND_FWD_2; navStartTime = millis();
            }
            break;
        case NAV_NX_BLIND_FWD_2:
            navSpeedCmd = navSpeedNormal; navTurnCmd = 0; navDirCmd = 1;
            if (millis() - navStartTime > NX_TIME_FWD_2) {
                navState = NAV_NX_BLIND_HIT_FWD; navStartTime = millis(); nexusAttackCount = 0;
            }
            break;
        case NAV_NX_BLIND_HIT_FWD:
            navSpeedCmd = 55; navTurnCmd = 0; navDirCmd = 1;
            if (millis() - navStartTime > NX_HIT_FWD_TIME) {
                navState = NAV_NX_BLIND_HIT_BCK; navStartTime = millis();
            }
            break;
        case NAV_NX_BLIND_HIT_BCK:
            navSpeedCmd = 55; navTurnCmd = 0; navDirCmd = -1;
            if (millis() - navStartTime > NX_HIT_BCK_TIME) {
                nexusAttackCount++;
                if (nexusAttackCount >= NX_HIT_COUNT) {
                    navState = NAV_RETREAT;
                } else {
                    navState = NAV_NX_BLIND_HIT_FWD; navStartTime = millis();
                }
            }
            break;

        case NAV_RETREAT:
            navSpeedCmd = navSpeedSlow;
            navTurnCmd = 0;
            navDirCmd = -1;
            
            static unsigned long retreatStart = 0;
            if (retreatStart == 0) retreatStart = millis();
            if (millis() - retreatStart > 1500) {
                retreatStart = 0;
                navState = NAV_IDLE;
                currentTask = TASK_NONE;
            }
            break;
            
        case NAV_ERROR:
            navSpeedCmd = 0;
            navTurnCmd = 0;
            break;
            
        default:
            navState = NAV_IDLE;
            break;
    }
}

// ============ Stop Navigation ============
void navStop() {
    navState = NAV_IDLE;
    currentTask = TASK_NONE;
    navSpeedCmd = 0;
    navTurnCmd = 0;
    nexusAttackCount = 0;
    waypointCount = 0;
    currentWaypointIndex = 0;
    arrivalCounter = 0;
    tofAvoid.active = false;
}

// ============ Task: Go to Single Coordinate ============
void taskGotoSingle(int x, int y) {
    currentTask = TASK_GOTO_SINGLE;
    waypointCount = 0;
    currentWaypointIndex = 0;
    arrivalCounter = 0;
    headingInitialized = false;
    
    int fromX = vive.valid ? vive.x : (FIELD_X_MIN + FIELD_X_MAX) / 2;
    int fromY = vive.valid ? vive.y : FIELD_Y_MIN + 500;
    
    Serial.println("========================================");
    Serial.printf("[NAV] taskGotoSingle: (%d,%d) -> (%d,%d)\n", fromX, fromY, x, y);
    
    // Check if a detour is necessary
    if (!isPathCrossingAnyObstacle(fromX, fromY, x, y)) {
        // nonstop
        pushWaypoint(x, y, true, SPEED_NORMAL);
    } else {
        // Attempt intelligent path
        if (!findSmartPath(fromX, fromY, x, y)) {
            // Alternative corridor route
            planCorridorPath(fromX, fromY, x, y);
        }
    }
    
    if (waypointCount > 0) {
        currentTarget = waypoints[0];
        navState = NAV_TO_WAYPOINT;
        taskStartTime = millis();
        navStartTime = millis();
        
        for (int i = 0; i < waypointCount; i++) {
            Serial.printf("  [%d] (%d,%d)%s\n", 
                          i+1, waypoints[i].x, waypoints[i].y,
                          waypoints[i].isFinalTarget ? " [Final target]" : "");
        }
    } else {
        navState = NAV_ERROR;
    }
    Serial.println("========================================");
}

// ============ Task: Smart Navigation ============
void taskGotoSmart(int x, int y) {
    // 使用同样的逻辑
    taskGotoSingle(x, y);
    currentTask = TASK_GOTO_SMART;
}

// ============ Task: Go to Three Coordinates ============
void taskGotoThree(int x1, int y1, int x2, int y2, int x3, int y3) {
    currentTask = TASK_GOTO_THREE;
    waypointCount = 0;
    currentWaypointIndex = 0;
    arrivalCounter = 0;
    
    pushWaypoint(x1, y1, false, SPEED_NORMAL);
    pushWaypoint(x2, y2, false, SPEED_NORMAL);
    pushWaypoint(x3, y3, true, SPEED_NORMAL);
    
    currentTarget = waypoints[0];
    navState = NAV_TO_WAYPOINT;
    taskStartTime = millis();
    navStartTime = millis();
}

// ============ Task: Capture High Tower ============
void taskCaptureHighTower() {
    currentTask = TASK_CAPTURE_HIGH;
    navState = NAV_HT_BLIND_FORWARD;
    taskStartTime = millis(); 
    navStartTime = millis();
    
    Serial.printf("[NAV] High Tower (Blind Mode): Team=%s\n", 
                  currentTeam == TEAM_RED ? "RED" : "BLUE");
}

// ============ Task: Capture Low Tower ============
void taskCaptureLowTower() {
    currentTask = TASK_CAPTURE_LOW;
    navState = NAV_LT_BLIND_FWD_1;
    taskStartTime = millis();
    navStartTime = millis();
    
    Serial.printf("[NAV] LT Blind: Start 5-Step Sequence\n");
}

// ============ Task: Attack Nexus ============
void taskAttackNexus() {
    currentTask = TASK_ATTACK_NEXUS;
    navState = NAV_NX_BLIND_FWD_1;
    taskStartTime = millis();
    navStartTime = millis();
    nexusAttackCount = 0;
    
    Serial.println("[NAV] Nexus Blind: Start Sequence");
}

// ============ Set Team ============
void setTeam(int team) {
    currentTeam = (team == TEAM_RED) ? TEAM_RED : TEAM_BLUE;
}

// ============ Status Functions ============
const char* getNavStateString() {
    switch (navState) {
        case NAV_IDLE: return "IDLE";
        case NAV_MANUAL: return "MANUAL";
        case NAV_WALL_FOLLOW: return "WALL_FOLLOW";
        case NAV_TO_WAYPOINT: return "TO_WAYPOINT";
        case NAV_APPROACH_TARGET: return "APPROACH";
        case NAV_ATTACKING: return "ATTACKING";
        case NAV_RETREAT: return "RETREAT";
        case NAV_HT_BLIND_FORWARD: return "HT_FWD";
        case NAV_HT_BLIND_TURN: return "HT_TURN";
        case NAV_HT_BLIND_HIT: return "HT_HIT";
        case NAV_TOF_AVOIDING: return "TOF_AVOID";
        case NAV_ERROR: return "ERROR";
        default: return "UNKNOWN";
    }
}

const char* getTaskString() {
    switch (currentTask) {
        case TASK_NONE: return "NONE";
        case TASK_GOTO_SINGLE: return "GOTO_1";
        case TASK_GOTO_THREE: return "GOTO_3";
        case TASK_GOTO_SMART: return "SMART_NAV";
        case TASK_CAPTURE_LOW: return "LOW_TOWER";
        case TASK_CAPTURE_HIGH: return "HIGH_TOWER";
        case TASK_ATTACK_NEXUS: return "NEXUS";
        case TASK_WALL_FOLLOW_FULL: return "WALL_FULL";
        case TASK_COVER_VIVE_LOCS: return "COVER_LOCS";
        default: return "UNKNOWN";
    }
}

bool isNavActive() {
    return (navState != NAV_IDLE && navState != NAV_ERROR);
}

int getWaypointProgress() {
    return currentWaypointIndex + 1;
}

int getTotalWaypoints() {
    return waypointCount;
}

int getViveQuality() {
    return vive.quality;
}

bool isViveAReliable() {
    return vive.sensorA.valid && vive.sensorA.quality >= 3;
}

bool isViveBReliable() {
    return vive.sensorB.valid && vive.sensorB.quality >= 3;
}

int getNexusAttackProgress() {
    return nexusAttackCount;
}

int getFinalTargetX() {
    return finalTargetX;
}

int getFinalTargetY() {
    return finalTargetY;
}

#endif
