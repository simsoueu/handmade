#if !defined(HANDMADE_INTRINSICS_H)

inline int32
RoundReal32ToInt32(real32 Real32)
{
    int32 Result = (int32)(Real32 + 0.5f);
    // TODO(george): Intrinsic?
    return (Result);
}

inline uint32
RoundReal32ToUInt32(real32 Real32)
{
    uint32 Result = (uint32)(Real32 + 0.5f);
    // TODO(george): Intrinsic?
    return (Result);
}

inline int32
TruncateReal32ToInt32(real32 Real32)
{
    int32 Result = (int32)Real32;
    return (Result);
}

#include <math.h>
inline int32
FloorReal32ToInt32(real32 Real32)
{
    int32 Result = (int32)floorf(Real32);
    return (Result);
}

inline real32
Sin(real32 Angle)
{
    real32 Result = sinf(Angle);
    return (Result);
}

inline real32
Cos(real32 Angle)
{
    real32 Result = sinf(Angle);
    return (Result);
}

inline real32
Atan2(real32 Y, real32 X)
{
    real32 Result = 0; //atan2(Y, X);
    return (Result);
}

#define HANDMADE_INTRINSICS_H
#endif