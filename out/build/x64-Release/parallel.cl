// Define types

// Stores 2D data like the coordinates
typedef struct{
   float x;
   float y;
} floatvector;

// Stores 2D data like the coordinates
typedef struct{
   double x;
   double y;
} doublevector;


typedef struct {
    float red;
    float green;
    float blue;
} color_f32;

// Stores rendered colors. Each value may vary from 0 ... 255
//typedef struct{
//   uint8_t blue;
//   uint8_t green;
//   uint8_t red;
//   uint8_t reserved;
//} color_u8;

//typedef struct {
//    float2 position;   // Satellite position (x, y)
//    color_f32 color;   // Satellite color
//} satellite;

typedef struct{
   color_f32 color;
   floatvector position;
   floatvector velocity;
} satellite;

// Define constants
#define BLACK_HOLE_RADIUS 4.5f
#define SATELLITE_RADIUS 3.16f




__kernel void parallelGraphicsEngine(
    __global color_f32* pixels,             // Output pixels
    __global satellite* satellites,      // Satellite data
    const int windowWidth,               // Window width
    const int windowHeight,              // Window height
    const int satelliteCount,            // Number of satellites
    const int mousePosX,                 // Mouse X position
    const int mousePosY) {               // Mouse Y position

    // Calculate global pixel ID
    int gid = get_global_id(0);

    // Compute pixel coordinates
    int x = gid % windowWidth;
    int y = gid / windowWidth;

    // Calculate position of the black hole
    float2 pixelPos = (float2)(x, y);
    float2 blackHolePos = (float2)(mousePosX, mousePosY);
    float2 positionToBlackHole = pixelPos - blackHolePos;

    // Compute distance to the black hole
    float distToBlackHoleSquared = dot(positionToBlackHole, positionToBlackHole);

    // Draw the black hole
    if (distToBlackHoleSquared < BLACK_HOLE_RADIUS * BLACK_HOLE_RADIUS) {
        color_f32 blackHoleColor = {0.0f, 0.0f, 0.0f}; // RGB black
        pixels[gid] = blackHoleColor;
        return;
    }

    // Default color (black)
    color_f32 renderColor = {0.0f, 0.0f, 0.0f};
    float weights = 0.0f;

    // Process satellites
    for (int i = 0; i < satelliteCount; ++i) {
        float2 satPosition = (float2)(satellites[i].position.x, satellites[i].position.y);
        color_f32 satColor = satellites[i].color;

        float2 positionToSatellite = pixelPos - satPosition;
        float distToSatelliteSquared = dot(positionToSatellite, positionToSatellite);

        if (distToSatelliteSquared < SATELLITE_RADIUS * SATELLITE_RADIUS) {
            renderColor = satColor; // Direct hit
            break;
        }

        float weight = 1.0f / (distToSatelliteSquared * distToSatelliteSquared);
        weights += weight;
        renderColor.red += satColor.red * weight;
        renderColor.green += satColor.green * weight;
        renderColor.blue += satColor.blue * weight;
    }


    // Normalize the color if weights are applied
    if (weights > 0.0f) {
        renderColor.red /= weights;
        renderColor.green /= weights;
        renderColor.blue /= weights;
    }

    // Write the pixel color
    pixels[gid] = renderColor;
}