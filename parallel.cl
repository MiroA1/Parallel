

__kernel void parallelGraphicsEngine(
    __global uchar4 *pixels,           
    __global float2 *satellitePositions, 
    __global float4 *satelliteColors,   
    int windowWidth,                   
    int windowHeight,                  
    int satelliteCount,                
    int mousePosX,                     
    int mousePosY,                     
    float blackHoleRadius,             
    float satelliteRadius              
)
{

    int pixelX = get_global_id(0);
    int pixelY = get_global_id(1);

    int i = pixelX + windowWidth * pixelY;

    // Initialize pixel color
    float4 renderColor = (float4)(0.0f, 0.0f, 0.0f, 0.0f);


    float2 positionToBlackHole = (float2)(pixelX - mousePosX, pixelY - mousePosY);
    float distToBlackHoleSquared = dot(positionToBlackHole, positionToBlackHole);

    if (distToBlackHoleSquared < blackHoleRadius * blackHoleRadius) {
        // Black hole pixels are black
        pixels[i] = (uchar4)(0, 0, 0, 255);
        return;
    }

    float shortestDistance = INFINITY;
    float weights = 0.0f;
    int hitsSatellite = 0;

    // Find closest satellite
    for (int j = 0; j < satelliteCount; ++j) {
        float2 satellitePos = satellitePositions[j]; 
        float2 difference = (float2)(pixelX - satellitePos.x, pixelY - satellitePos.y);
        float distance = length(difference);


        if (distance < satelliteRadius) {
            // Pixel is inside a satellite
            renderColor.x = 1.0f;  
            renderColor.y = 1.0f;
            renderColor.z = 1.0f;

            hitsSatellite = 1;
            break;

        } else {
            float weight = 1.0f / (distance * distance * distance * distance);
            weights += weight;

            if (distance < shortestDistance) {
                shortestDistance = distance;
                renderColor = satelliteColors[j];  
            }
        }
    }

    // If no satellite was hit, apply weighted blending
    if (!hitsSatellite) {
        for (int k = 0; k < satelliteCount; ++k) {
            float2 satellitePos = satellitePositions[k];
            float2 difference = (float2)(pixelX - satellitePos.x, pixelY - satellitePos.y);
            float dist2 = dot(difference, difference);
            float weight = 1.0f / (dist2 * dist2);

            renderColor.x += (satelliteColors[k].x * weight / weights) * 3.0f;
            renderColor.y += (satelliteColors[k].y * weight / weights) * 3.0f;
            renderColor.z += (satelliteColors[k].z * weight / weights) * 3.0f;
        }
    }


    // Convert color to 8-bit and write to output
    pixels[i] = (uchar4)((uchar)(clamp(renderColor.x, 0.0f, 1.0f) * 255.0f),
                         (uchar)(clamp(renderColor.y, 0.0f, 1.0f) * 255.0f),
                         (uchar)(clamp(renderColor.z, 0.0f, 1.0f) * 255.0f),
                         255);
}