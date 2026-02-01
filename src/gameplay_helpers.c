#include "gameplay_helpers.h"
#include "../raylib/src/raymath.h"
#include <float.h>

bool CheckCollisionCircleRotatedRect(Vector2 center, float radius, Rectangle rect, float rotation) {
    // 1. Transform Circle Center to Local Space of the Rectangle
    Vector2 rectCenter = { rect.x + rect.width / 2.0f, rect.y + rect.height / 2.0f };
    
    // Vector from rect center to circle center
    Vector2 dir = Vector2Subtract(center, rectCenter);
    
    // Rotate this vector by -rotation (inverse rotation)
    // Create rotation matrix / or just manual sin/cos
    float rad = -rotation * DEG2RAD;
    float s = sinf(rad);
    float c = cosf(rad);
    
    Vector2 localDir = {
        dir.x * c - dir.y * s,
        dir.x * s + dir.y * c
    };
    
    // Local Circle Center
    Vector2 localCenter = Vector2Add(rectCenter, localDir); // Wait, no.
    
    // Actually, simpler:
    // Just rotate the point (center) around (rectCenter) by -rotation.
    // The AABB is axis aligned at 0 rotation centered at rectCenter.
    // Wait, rect.x/y is top-left usually.
    // Correct logic:
    // 1. Center of rect
    Vector2 origin = { rect.x + rect.width/2.0f, rect.y + rect.height/2.0f };
    
    // 2. Rotate 'center' around 'origin' by -rotation
    Vector2 diff = Vector2Subtract(center, origin);
    float r = -rotation * DEG2RAD;
    float cosA = cosf(r);
    float sinA = sinf(r);
    
    Vector2 localPos = {
        diff.x * cosA - diff.y * sinA,
        diff.x * sinA + diff.y * cosA
    };
    
    // 3. Now check AABB collision with localPos vs unrotated Rect centered at (0,0) with dims width/height
    // Since we rotated around center, the local rect is centered at (0,0)
    float halfW = rect.width / 2.0f;
    float halfH = rect.height / 2.0f;
    
    // Clamp point to box
    float closestX = fmaxf(-halfW, fminf(localPos.x, halfW));
    float closestY = fmaxf(-halfH, fminf(localPos.y, halfH));
    
    Vector2 closest = { closestX, closestY };
    
    // Check distance
    float distance = Vector2Distance(localPos, closest);
    
    return distance < radius;
}


int Gameplay_GetClosestDoor(const Level *level, Vector2 position) {
    if (!level) return -1;
    int closest = -1;
    float minDistSq = FLT_MAX;
    
    for (int i = 0; i < level->doorCount; i++) {
        Rectangle r = level->doors[i].rect;
        Vector2 center = { r.x + r.width/2.0f, r.y + r.height/2.0f };
        float d = Vector2DistanceSqr(position, center);
        if (d < minDistSq) {
            minDistSq = d;
            closest = i;
        }
    }
    return closest;
}

Vector2 Gameplay_GetRayHit(Vector2 start, Vector2 end, const Level *level) {
    if (!level) return end;

    Vector2 closestHit = end;
    float closestDistSq = Vector2DistanceSqr(start, end);

    // Walls
    for (int i = 0; i < level->wallCount; i++) {
        // Simple AABB raycast if rotation is 0, else OBB
        // For now, let's just do segments of the OBB?
        // Or transform ray to local space?
        
        Wall w = level->walls[i];
        
        // Transform Ray to Local Space of Wall
        Vector2 origin = { w.rect.x + w.rect.width/2.0f, w.rect.y + w.rect.height/2.0f };
        float r = -w.rotation * DEG2RAD;
        float c = cosf(r);
        float s = sinf(r);
        
        // Transform Start
        Vector2 dS = Vector2Subtract(start, origin);
        Vector2 localStart = { dS.x * c - dS.y * s, dS.x * s + dS.y * c };
        
        // Transform End (Using Direction)
        Vector2 dE = Vector2Subtract(end, origin); // Actually we need ray direction
        // Let's just transform both points
        Vector2 localEnd = { dE.x * c - dE.y * s, dE.x * s + dE.y * c };
        
        // Local Rect (Centered at 0,0)
        Rectangle localRectIndices = { -w.rect.width/2.0f, -w.rect.height/2.0f, w.rect.width, w.rect.height };
        
        // Define standard 4 segments of AABB
        Vector2 p1 = { localRectIndices.x, localRectIndices.y };
        Vector2 p2 = { localRectIndices.x + localRectIndices.width, localRectIndices.y };
        Vector2 p3 = { localRectIndices.x + localRectIndices.width, localRectIndices.y + localRectIndices.height };
        Vector2 p4 = { localRectIndices.x, localRectIndices.y + localRectIndices.height };
        
        // Check intersections with ray (localStart -> localEnd)
        Vector2 hit;
        
        if (CheckCollisionLines(localStart, localEnd, p1, p2, &hit)) {
            // Transform hit back to world?
            // Easier: just check distance in local space? Yes.
            float d2 = Vector2DistanceSqr(localStart, hit);
            if (d2 < closestDistSq) { 
                // Transform hit back
                /* 
                   Local = Rot(-A) * (World - Origin)
                   World - Origin = Rot(A) * Local
                   World = Origin + Rot(A) * Local
                */
                // float backR = w.rotation * DEG2RAD; ...
                // Actually, let's just update valid closestDistSq
                closestDistSq = d2;
                
                // Calculate world hit
                 float r2 = w.rotation * DEG2RAD;
                 float c2 = cosf(r2);
                 float s2 = sinf(r2);
                 closestHit.x = origin.x + (hit.x * c2 - hit.y * s2);
                 closestHit.y = origin.y + (hit.x * s2 + hit.y * c2);
            }
        }
        if (CheckCollisionLines(localStart, localEnd, p2, p3, &hit)) {
             float d2 = Vector2DistanceSqr(localStart, hit);
             if (d2 < closestDistSq) {
                 closestDistSq = d2;
                 float r2 = w.rotation * DEG2RAD;
                 float c2 = cosf(r2);
                 float s2 = sinf(r2);
                 closestHit.x = origin.x + (hit.x * c2 - hit.y * s2);
                 closestHit.y = origin.y + (hit.x * s2 + hit.y * c2);
             }
        }
        if (CheckCollisionLines(localStart, localEnd, p3, p4, &hit)) {
             float d2 = Vector2DistanceSqr(localStart, hit);
             if (d2 < closestDistSq) {
                 closestDistSq = d2;
                 float r2 = w.rotation * DEG2RAD;
                 float c2 = cosf(r2);
                 float s2 = sinf(r2);
                 closestHit.x = origin.x + (hit.x * c2 - hit.y * s2);
                 closestHit.y = origin.y + (hit.x * s2 + hit.y * c2);
             }
        }
        if (CheckCollisionLines(localStart, localEnd, p4, p1, &hit)) {
            float d2 = Vector2DistanceSqr(localStart, hit);
             if (d2 < closestDistSq) {
                 closestDistSq = d2;
                 float r2 = w.rotation * DEG2RAD;
                 float c2 = cosf(r2);
                 float s2 = sinf(r2);
                 closestHit.x = origin.x + (hit.x * c2 - hit.y * s2);
                 closestHit.y = origin.y + (hit.x * s2 + hit.y * c2);
             }
        }
    }

    // Doors (Keep as AABB for now or update similarly?)
    // Doors in level struct are `Door doors[MAX]`. 
    // They have simple `rect`.
    // Assuming Doors don't rotate for now? Or do they?
    // "add a wall struct and rearrange" -> mostly walls.
    // Let's keep doors as AABB CheckCollisionLines for now.
    for (int i = 0; i < level->doorCount; i++) {
        Rectangle rec = level->doors[i].rect;
            int doorIdx = i; // ...
            if (level->doors[doorIdx].isOpen) continue;
            
             // AABB segments
             Vector2 p1 = { rec.x, rec.y };
             Vector2 p2 = { rec.x + rec.width, rec.y };
             Vector2 p3 = { rec.x + rec.width, rec.y + rec.height };
             Vector2 p4 = { rec.x, rec.y + rec.height };
             
             Vector2 hit;
             if (CheckCollisionLines(start, end, p1, p2, &hit)) {
                 if (Vector2DistanceSqr(start, hit) < closestDistSq) { closestHit = hit; closestDistSq = Vector2DistanceSqr(start, hit); }
             }
             if (CheckCollisionLines(start, end, p2, p3, &hit)) {
                 if (Vector2DistanceSqr(start, hit) < closestDistSq) { closestHit = hit; closestDistSq = Vector2DistanceSqr(start, hit); }
             }
             if (CheckCollisionLines(start, end, p3, p4, &hit)) {
                 if (Vector2DistanceSqr(start, hit) < closestDistSq) { closestHit = hit; closestDistSq = Vector2DistanceSqr(start, hit); }
             }
             if (CheckCollisionLines(start, end, p4, p1, &hit)) {
                 if (Vector2DistanceSqr(start, hit) < closestDistSq) { closestHit = hit; closestDistSq = Vector2DistanceSqr(start, hit); }
             }
    }

    return closestHit;
}
