/**
 * Transform class which represents a 3D transformation in space.
 * It includes translation, rotation, and scaling operations.
 */

#pragma once

#include "matrix4.h"
#include "vec3.h"
#include "vec4.h"

namespace ME {

/**
 * Transform class for handling 3D transformations.
 * Contains position, rotation, and scale.
 * Use this to manipulate the position, rotation, and scale of objects in 3D space.
 * Will return the transformation matrix / model matrix.
 */
class Transform {
   public:
    Transform();
    ~Transform();

    inline ME::Vec3 GetPosition() const {
        return position;
    }
    void SetPosition(float x, float y, float z);
    // For 2D transforms, set z to 0.0f.
    void SetPosition(float x, float y);

    void SetPosition(const ME::Vec3& pos);

    inline ME::Vec4 GetRotation() const {
        return rotation;
    }
    void SetRotation(float x, float y, float z, float w = 1.0f);

    inline ME::Vec3 GetScale() const {
        return scale;
    }
    void SetScale(float x, float y, float z);
    // For 2D transforms, set z to 1.0f.
    void SetScale(float x, float y);
    // This will set the scale to the same value for all axes.
    void SetScale(float scale);
    void SetScale(const ME::Vec3& scale);

    // Translation * Rotation * Scale. With rotation only about z (x and y are 0) it is filled in closed form, with no
    // trig at angle 0; the result equals the general product.
    ME::Mat4 GetModelMatrix() const;

    // True after any setter since the last ClearDirty(); a new transform starts dirty. Renderers rebuild the model
    // matrix only for dirty transforms.
    bool IsDirty() const {
        return bDirty;
    }
    void ClearDirty() {
        bDirty = false;
    }

   private:
    ME::Vec3 position;
    ME::Vec4 rotation;
    ME::Vec3 scale;
    bool bDirty = true;
};

}  // namespace ME
