#ifndef FIELD_CONFIG_H
#define FIELD_CONFIG_H

// ============ Vive Unit Conversion ============
#define VIVE_UNITS_PER_INCH 40.0

// ============ Field Boundaries ============
#define FIELD_X_MIN 3200
#define FIELD_X_MAX 5130
#define FIELD_Y_MIN 1990
#define FIELD_Y_MAX 6380

// ============ Safe Navigation Zone ============
#define VIVE_SAFE_X_MARGIN_INCH 10.0
#define VIVE_SAFE_Y_MARGIN_INCH 30.0
#define VIVE_SAFE_X_MIN (FIELD_X_MIN + (int)(VIVE_SAFE_X_MARGIN_INCH * VIVE_UNITS_PER_INCH))
#define VIVE_SAFE_X_MAX (FIELD_X_MAX - (int)(VIVE_SAFE_X_MARGIN_INCH * VIVE_UNITS_PER_INCH))
#define VIVE_SAFE_Y_MIN (FIELD_Y_MIN + (int)(VIVE_SAFE_Y_MARGIN_INCH * VIVE_UNITS_PER_INCH))
#define VIVE_SAFE_Y_MAX (FIELD_Y_MAX - (int)(VIVE_SAFE_Y_MARGIN_INCH * VIVE_UNITS_PER_INCH))

// ============ Field Dimensions ============
#define FIELD_WIDTH_INCH 60
#define FIELD_LENGTH_INCH 144

// ============ Ramp Area ============
#define RAMP_X_MIN 3200
#define RAMP_X_MAX 3600
#define RAMP_ENTRY_Y 1990
#define RAMP_EXIT_Y 6380
#define RAMP_PLATFORM_Y_MIN 3200
#define RAMP_PLATFORM_Y_MAX 5100
#define RAMP_HEIGHT_INCH 9.0
#define RAMP_ANGLE_DEG 13.0
#define RAMP_LENGTH_INCH 37.0

#define RAMP_ENTRY_X 3400
#define RAMP_ENTRY_APPROACH_Y (RAMP_ENTRY_Y - 200)

// ============ High Tower Position ============
#define HIGH_TOWER_X 3200
#define HIGH_TOWER_CENTER_Y 4796
#define HIGH_TOWER_BLUE_Y 4412
#define HIGH_TOWER_RED_Y 5180

#define HIGH_TOWER_APPROACH_X (HIGH_TOWER_X + (int)(10.0 * VIVE_UNITS_PER_INCH))
#define HIGH_TOWER_ATTACK_X (HIGH_TOWER_X + (int)(3.0 * VIVE_UNITS_PER_INCH))

// ============ Low Tower Position ============
#define LOW_TOWER_X 4452
#define LOW_TOWER_CENTER_Y 4124
#define LOW_TOWER_BLUE_Y 3920
#define LOW_TOWER_RED_Y 4170

#define LOW_TOWER_APPROACH_X (LOW_TOWER_X + (int)(12.0 * VIVE_UNITS_PER_INCH))
#define LOW_TOWER_ATTACK_X (LOW_TOWER_X + (int)(4.0 * VIVE_UNITS_PER_INCH))
// ============ Low Tower Obstacle Polygon (clockwise) ============
#define LOW_TOWER_P1_X 4676
#define LOW_TOWER_P1_Y 3908
#define LOW_TOWER_P2_X 4568
#define LOW_TOWER_P2_Y 4182
#define LOW_TOWER_P3_X 4784
#define LOW_TOWER_P3_Y 4312
#define LOW_TOWER_P4_X 4344
#define LOW_TOWER_P4_Y 4054

#define ENABLE_NAV_TIMEOUT 0

// Additional safety margin(inch -> vive)
#define LOW_TOWER_MARGIN_INCH 6.0
#define LOW_TOWER_MARGIN_VIVE (int)(LOW_TOWER_MARGIN_INCH * VIVE_UNITS_PER_INCH)

// Point lies within the convex quadrilateral (using the same-direction cross product, assuming the four points are given in clockwise or counterclockwise order)
inline long cross2d(long ax, long ay, long bx, long by) { return ax*by - ay*bx; }

inline bool isInLowTowerPoly(int x, int y, int marginVive = LOW_TOWER_MARGIN_VIVE) {
  // Simplified approach: First, use AABB + margin to quickly eliminate
  int minX = min(min(LOW_TOWER_P1_X, LOW_TOWER_P2_X), min(LOW_TOWER_P3_X, LOW_TOWER_P4_X)) - marginVive;
  int maxX = max(max(LOW_TOWER_P1_X, LOW_TOWER_P2_X), max(LOW_TOWER_P3_X, LOW_TOWER_P4_X)) + marginVive;
  int minY = min(min(LOW_TOWER_P1_Y, LOW_TOWER_P2_Y), min(LOW_TOWER_P3_Y, LOW_TOWER_P4_Y)) - marginVive;
  int maxY = max(max(LOW_TOWER_P1_Y, LOW_TOWER_P2_Y), max(LOW_TOWER_P3_Y, LOW_TOWER_P4_Y)) + marginVive;
  if (x < minX || x > maxX || y < minY || y > maxY) return false;

  /*Perform the test for convex polygons in the same direction (not actually "expanding outward",
  but combined with AABB margin, it is sufficient for practical use and stable performance) */
  const int px[4] = {LOW_TOWER_P1_X, LOW_TOWER_P2_X, LOW_TOWER_P3_X, LOW_TOWER_P4_X};
  const int py[4] = {LOW_TOWER_P1_Y, LOW_TOWER_P2_Y, LOW_TOWER_P3_Y, LOW_TOWER_P4_Y};

  long prev = 0;
  for (int i=0;i<4;i++){
    int j=(i+1)%4;
    long ex = px[j]-px[i];
    long ey = py[j]-py[i];
    long vx = x - px[i];
    long vy = y - py[i];
    long c = cross2d(ex, ey, vx, vy);
    if (c == 0) continue;
    if (prev == 0) prev = c;
    else if ((prev > 0) != (c > 0)) return false;
  }
  return true;
}


// ============ Nexus Positions ============
#define NEXUS_X 4452
#define NEXUS_1_Y 1900
#define NEXUS_2_Y 6400
#define NEXUS_ATTACK_RADIUS_INCH 16.5
#define NEXUS_ATTACK_HEIGHT_INCH 7.5
#define NEXUS_ROTATION_RPS 0.1

#define NEXUS_APPROACH_DIST_INCH 20.0
#define NEXUS_ATTACK_DIST_INCH 8.0

// ============ Start Zone ============
#define START_ZONE_X_MIN 3200
#define START_ZONE_X_MAX 5130
#define START_ZONE_BLUE_Y 6380
#define START_ZONE_RED_Y 1990

// ============ Robot Dimensions ============
#define CAR_WIDTH_INCH 6.0
#define CAR_LENGTH_INCH 7.2
#define CAR_HALF_WIDTH (CAR_WIDTH_INCH / 2.0)
#define CAR_HALF_LENGTH (CAR_LENGTH_INCH / 2.0)

// ============ Safety Margins ============
#define WALL_MARGIN_INCH 4.0
#define OBSTACLE_MARGIN_INCH 6.0
#define BUTTON_APPROACH_DIST_INCH 2.0

// ============ Navigation Tolerances ============
#define POSITION_TOLERANCE 100
#define POSITION_TOLERANCE_FINE 100
#define HEADING_TOLERANCE 0.15
#define HEADING_TOLERANCE_FINE 0.08

// ============ Vive Signal Quality ============
#define VIVE_SIGNAL_MIN_QUALITY 3
#define VIVE_POSITION_JUMP_THRESHOLD 400
#define VIVE_INVALID_COORD 0

// ============ Speed Settings ============
#define AUTO_SPEED_FAST 70
#define AUTO_SPEED_NORMAL 60
#define AUTO_SPEED_SLOW 35
#define AUTO_SPEED_APPROACH 25
#define AUTO_SPEED_ATTACK 50
#define AUTO_SPEED_RAMP_UP 60
#define AUTO_SPEED_RAMP_DOWN 50

// ============ Turn Settings ============
#define TURN_SPEED_NORMAL 40
#define TURN_SPEED_SHARP 60
#define TURN_SPEED_FINE 20

// ============ Timeout Settings ============
#define BUTTON_HOLD_TIME 15000
#define AUTONOMOUS_TIMEOUT 30000
#define WAYPOINT_TIMEOUT 10000
#define NAVIGATION_TIMEOUT 15000
#define ATTACK_TIMEOUT 12000
#define VIVE_TIMEOUT 2000

// ============ Nexus Attack Settings ============
#define NEXUS_ATTACK_COUNT 4
#define NEXUS_ATTACK_FORWARD_TIME 400
#define NEXUS_ATTACK_BACKWARD_TIME 300

// ============ Team Definitions ============
#define TEAM_BLUE 0
#define TEAM_RED 1

// ============ TOF Parameters ============
#define TOF_OBSTACLE_THRESHOLD 300
#define TOF_WALL_TARGET 180
#define TOF_WALL_TOLERANCE 30
#define TOF_VALID_MIN 30
#define TOF_VALID_MAX 2000
#define TOF_CLOSE_THRESHOLD 100

// ============ Navigation Control Parameters ============
#define NAV_KP_DISTANCE 0.008
#define NAV_KP_HEADING 35.0
#define NAV_MAX_TURN 50

// ============ Helper Functions ============

inline float viveToInch(int viveUnits) {
    return viveUnits / VIVE_UNITS_PER_INCH;
}

inline int inchToVive(float inches) {
    return (int)(inches * VIVE_UNITS_PER_INCH);
}

inline bool isInField(int x, int y) {
    return (x >= FIELD_X_MIN && x <= FIELD_X_MAX &&
            y >= FIELD_Y_MIN && y <= FIELD_Y_MAX);
}

inline bool isInViveSafeZone(int x, int y) {
    return (x >= VIVE_SAFE_X_MIN && x <= VIVE_SAFE_X_MAX &&
            y >= VIVE_SAFE_Y_MIN && y <= VIVE_SAFE_Y_MAX);
}

inline bool isOnRampX(int x) {
    return (x >= RAMP_X_MIN && x <= RAMP_X_MAX);
}

inline bool isOnRamp(int x, int y) {
    return (x >= RAMP_X_MIN && x <= RAMP_X_MAX &&
            y >= RAMP_ENTRY_Y && y <= RAMP_EXIT_Y);
}

inline bool isOnRampPlatform(int x, int y) {
    return (x >= RAMP_X_MIN && x <= RAMP_X_MAX &&
            y >= RAMP_PLATFORM_Y_MIN && y <= RAMP_PLATFORM_Y_MAX);
}

inline bool isOnRampEntry(int x, int y) {
    return (x >= RAMP_X_MIN && x <= RAMP_X_MAX &&
            y >= RAMP_ENTRY_Y && y < RAMP_PLATFORM_Y_MIN);
}

inline bool isOnRampExit(int x, int y) {
    return (x >= RAMP_X_MIN && x <= RAMP_X_MAX &&
            y > RAMP_PLATFORM_Y_MAX && y <= RAMP_EXIT_Y);
}

inline bool isNearNexus(int x, int y, float marginInch = 3.0) {
    int nexusRadiusVive = inchToVive(NEXUS_ATTACK_RADIUS_INCH + marginInch);
    int dx1 = x - NEXUS_X;
    int dy1 = y - NEXUS_1_Y;
    if (dx1*dx1 + dy1*dy1 < nexusRadiusVive*nexusRadiusVive) return true;
    int dx2 = x - NEXUS_X;
    int dy2 = y - NEXUS_2_Y;
    if (dx2*dx2 + dy2*dy2 < nexusRadiusVive*nexusRadiusVive) return true;
    return false;
}

inline float distanceBetween(int x1, int y1, int x2, int y2) {
    float dx = x2 - x1;
    float dy = y2 - y1;
    return sqrt(dx*dx + dy*dy);
}

inline float headingTo(int fromX, int fromY, int toX, int toY) {
    return atan2(toX - fromX, toY - fromY);
}

inline float normalizeAngle(float angle) {
    while (angle > PI) angle -= 2*PI;
    while (angle < -PI) angle += 2*PI;
    return angle;
}

inline void getLowTowerButtonPos(int team, int& x, int& y) {
    x = LOW_TOWER_X;
    y = (team == TEAM_BLUE) ? LOW_TOWER_BLUE_Y : LOW_TOWER_RED_Y;
}

inline void getHighTowerButtonPos(int team, int& x, int& y) {
    x = HIGH_TOWER_X;
    y = (team == TEAM_BLUE) ? HIGH_TOWER_BLUE_Y : HIGH_TOWER_RED_Y;
}


inline void getEnemyNexusPos(int team, int& x, int& y) {
    x = NEXUS_X;
    y = (team == TEAM_RED) ? NEXUS_1_Y : NEXUS_2_Y;
}

inline void getLowTowerApproachPos(int team, int& x, int& y) {
    x = LOW_TOWER_APPROACH_X;
    y = (team == TEAM_BLUE) ? LOW_TOWER_BLUE_Y : LOW_TOWER_RED_Y;
}

inline void getHighTowerApproachPos(int team, int& x, int& y) {
    x = HIGH_TOWER_APPROACH_X;
    y = (team == TEAM_BLUE) ? HIGH_TOWER_BLUE_Y : HIGH_TOWER_RED_Y;
}

inline bool targetRequiresRamp(int targetX, int targetY) {
    return isOnRamp(targetX, targetY);
}

inline void getRampEntryPos(int& x, int& y) {
    x = RAMP_ENTRY_X;
    y = RAMP_ENTRY_APPROACH_Y;
}

#endif
