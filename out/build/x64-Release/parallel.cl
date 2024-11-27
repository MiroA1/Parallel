

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
    //int i = get_global_id(0); // Global thread index

    //if (i >= windowWidth * windowHeight) return;

    // Compute pixel position
    int pixelX = get_global_id(0);
    int pixelY = get_global_id(1);

    int i = pixelX + windowWidth * pixelY;

    // Initialize pixel color
    float4 renderColor = (float4)(0.0f, 0.0f, 0.0f, 0.0f);

    // Compute distance to the black hole
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
        float2 satellitePos = satellitePositions[j];  // Access satellite position from the buffer
        float2 difference = (float2)(pixelX - satellitePos.x, pixelY - satellitePos.y);
        float distance = length(difference);

        //if (i < 20) {
        //printf("Pixel (%d, %d) - Satellite %d: distance = %f, radius = %f\n", 
        //        pixelX, pixelY, j, distance, satelliteRadius);
        //}   


        if (distance < satelliteRadius) {
            // Pixel is inside a satellite
            renderColor.x = 1.0f;  // Access satellite color from the buffer
            renderColor.y = 1.0f;
            renderColor.z = 1.0f;

            hitsSatellite = 1;

            //if (i < 20) {
            //    printf("Pixel (%d, %d) - Satellite %d: Applying color: (%f, %f, %f)\n",
            //            pixelX, pixelY, j, renderColor.x, renderColor.y, renderColor.z);
            //}

            break;
        } else {
            float weight = 1.0f / (distance * distance * distance * distance);
            weights += weight;

            if (distance < shortestDistance) {
                shortestDistance = distance;
                renderColor = satelliteColors[j];  // Access satellite color from the buffer
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

    //Print debug information for the first few pixels
    //if (i < 20) {
    //    printf("Pixel (%d, %d): color(%f, %f, %f)\n", pixelX, pixelY, renderColor.x, renderColor.y, renderColor.z);
    //}

    // Convert color to 8-bit and write to output
    pixels[i] = (uchar4)((uchar)(clamp(renderColor.x, 0.0f, 1.0f) * 255.0f),
                         (uchar)(clamp(renderColor.y, 0.0f, 1.0f) * 255.0f),
                         (uchar)(clamp(renderColor.z, 0.0f, 1.0f) * 255.0f),
                         255);
}