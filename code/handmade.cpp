#include "handmade.h"
#include "handmade_intrinsics.h"

internal void
DrawRectangle(game_offscreen_buffer *Buffer,
              real32 RealMinX, real32 RealMinY, real32 RealMaxX, real32 RealMaxY,
              real32 R, real32 G, real32 B)
{
    int32 MinX = RoundReal32ToInt32(RealMinX);
    int32 MinY = RoundReal32ToInt32(RealMinY);
    int32 MaxX = RoundReal32ToInt32(RealMaxX);
    int32 MaxY = RoundReal32ToInt32(RealMaxY);

    if (MinX < 0)
    {
        MinX = 0;
    }

    if (MinY < 0)
    {
        MinY = 0;
    }

    if (MaxX > Buffer->Width)
    {
        MaxX = Buffer->Width;
    }

    if (MaxY > Buffer->Height)
    {
        MaxY = Buffer->Height;
    }

    uint32 Color = (RoundReal32ToUInt32(R * 255.0f) << 16) |
                   (RoundReal32ToUInt32(G * 255.0f) << 8) |
                   RoundReal32ToUInt32(B * 255.0f);

    uint8 *Row = (uint8 *)Buffer->Memory + MinX * Buffer->BytesPerPixel + MinY * Buffer->Pitch;

    for (int Y = MinY; Y < MaxY; Y++)
    {
        uint32 *Pixel = (uint32 *)Row;
        for (int X = MinX; X < MaxX; X++)
        {
            *Pixel++ = Color;
        }

        Row += Buffer->Pitch;
    }
}

inline tile_map *
GetTileMap(world *World, int32 TileMapX, int32 TileMapY)
{
    tile_map *TileMap = 0;

    if ((TileMapX >= 0) && (TileMapX < World->TileMapCountX) &&
        (TileMapY >= 0) && (TileMapX < World->TileMapCountY))
    {
        TileMap = &World->TileMaps[TileMapY * World->TileMapCountX + TileMapX];
    }

    return (TileMap);
}

inline bool32
GetTileValueUnchecked(world *World, tile_map *TileMap, int32 TileX, int32 TileY)
{
    Assert(TileMap);
    Assert((TileX >= 0) && (TileX < World->CountX) &&
           (TileY >= 0) && (TileY < World->CountY));

    uint32 TileMapValue = TileMap->Tiles[(TileY * World->CountX) + TileX];
    return (TileMapValue);
}

internal bool32
IsTileMapPointEmpty(world *World, tile_map *TileMap, int32 TestTileX, int32 TestTileY)
{
    bool32 Empty = false;

    if (TileMap)
    {
        if ((TestTileX >= 0) && (TestTileX < World->CountX) &&
            (TestTileY >= 0) && (TestTileY < World->CountY))
        {
            uint32 TileMapValue = GetTileValueUnchecked(World, TileMap, TestTileX, TestTileY);
            Empty = (TileMapValue == 0);
        }
    }

    return (Empty);
}

inline void
RecanonicalCoord(world *World, int32 TileCount, int32 *TileMap, int32 *Tile, real32 *TileRel)
{
    int32 offset = FloorReal32ToInt32(*TileRel / World->TileSizeInMeters);
    *Tile += offset;
    *TileRel -= offset * World->TileSizeInMeters;

    Assert(*TileRel >= 0);
    Assert(*TileRel < World->TileSizeInMeters);

    if (*Tile < 0) 
    {
        *Tile = TileCount + *Tile;
        --*TileMap;
        // int32 Delta = (-*Tile / TileCount) + 1;
        // *TileMap -= Delta;
        // *Tile = (TileCount + (*Tile % TileCount)) % TileCount;
    }

    if (*Tile >= TileCount)
    {
        *Tile = *Tile - TileCount;
        ++*TileMap;
        // *TileMap += *Tile / TileCount;
        // *Tile = *Tile % TileCount;
    }

    return;
}

inline world_position
RecanonicalPosition(world *World, world_position Pos)
{
    world_position Result = Pos;

    RecanonicalCoord(
        World, World->CountX, &Result.TileMapX, &Result.TileX, &Result.X);
    
    RecanonicalCoord(
        World, World->CountY, &Result.TileMapY, &Result.TileY, &Result.Y);

    return (Result);
}

internal bool32
IsWorldPointEmpty(world *World, world_position Pos)
{
    bool32 Empty = false;

    tile_map *TileMap = GetTileMap(World, Pos.TileMapX, Pos.TileMapY);
    
    // if (TileMap) 
    // {
    //     if (Pos.TileX >= 0 && Pos.TileX < World->TileMapCountX && Pos.TileY >= 0 && Pos.TileY < World->TileMapCountY)
    //     {
    //         uint32 TileValue = GetTileValueUnchecked(World, TileMap, Pos.TileX, Pos.TileY);

    //         Empty = TileValue == 0;
    //     }
    // }
    Empty = IsTileMapPointEmpty(World, TileMap, Pos.TileX, Pos.TileY);

    return (Empty);
}

internal void
GameOutputSound(game_state *GameState, game_sound_output_buffer *SoundBuffer, int ToneHz)
{
    int16 ToneVolume = 3000;
    int WavePeriod = SoundBuffer->SamplesPerSecond / ToneHz;

    int16 *SampleOut = SoundBuffer->Samples;
    for (int SampleIndex = 0; SampleIndex < SoundBuffer->SampleCount; SampleIndex++)
    {
#if 0
        real32 SineValue = sinf(GameState->tSine);
        int16 SampleValue = (int16)(SineValue * ToneVolume);
#else
        int16 SampleValue = 0;
#endif
        *SampleOut++ = SampleValue;
        *SampleOut++ = SampleValue;

#if 0
		GameState->tSine += 2.0f*Pi32*1.0f / (real32)WavePeriod;

        if (GameState->tSine > 2.0f*Pi32)
        {
            GameState->tSine -= 2.0f*Pi32;
        }
#endif
    }
}

extern "C" GAME_UPDATE_AND_RENDER(GameUpdateAndRender)
{
    Assert((&Input->Controllers[0].Start - &Input->Controllers[0].Buttons[0]) ==
           (ArrayCount(Input->Controllers[0].Buttons) - 1));
    Assert(sizeof(game_state) <= Memory->PermanentStorageSize);

#define TILE_MAP_COUNT_Y 9
#define TILE_MAP_COUNT_X 17
    uint32 Tiles00[TILE_MAP_COUNT_Y][TILE_MAP_COUNT_X] =
        {
            {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
            {1, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1},
            {1, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 1},
            {1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1},
            {1, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0},
            {1, 1, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 1},
            {1, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1},
            {1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1},
            {1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1}};

    uint32 Tiles01[TILE_MAP_COUNT_Y][TILE_MAP_COUNT_X] =
        {
            {1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1},
            {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
            {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
            {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
            {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
            {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
            {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
            {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
            {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}};

    uint32 Tiles10[TILE_MAP_COUNT_Y][TILE_MAP_COUNT_X] =
        {
            {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
            {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
            {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
            {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
            {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
            {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
            {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
            {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
            {1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1}};

    uint32 Tiles11[TILE_MAP_COUNT_Y][TILE_MAP_COUNT_X] =
        {
            {1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1},
            {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
            {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
            {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
            {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
            {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
            {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
            {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
            {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}};

    tile_map TileMaps[2][2];

    TileMaps[0][0].Tiles = (uint32 *)Tiles00;
    TileMaps[0][1].Tiles = (uint32 *)Tiles10;
    TileMaps[1][0].Tiles = (uint32 *)Tiles01;
    TileMaps[1][1].Tiles = (uint32 *)Tiles11;

    world World;

    World.TileMapCountX = 2;
    World.TileMapCountY = 2;
    World.CountX = TILE_MAP_COUNT_X;
    World.CountY = TILE_MAP_COUNT_Y;

    World.TileSizeInMeters = 1.4f;
    World.TileSizeInPixels = 60;
    World.MetersToPixels = (real32) World.TileSizeInPixels / (real32) World.TileSizeInMeters;

    World.LowerLeftX = -(real32)World.TileSizeInPixels / 2;
    World.LowerLeftY = (real32) Buffer->Height;

    real32 PlayerHeight = 1.4f;
    real32 PlayerWidth = 0.75f * PlayerHeight;
    

    World.TileMaps = (tile_map *)TileMaps;


    game_state *GameState = (game_state *)Memory->PermanentStorage;
    if (!Memory->IsInitialized)
    {
        GameState->PlayerPos = {};
        GameState->PlayerPos.TileMapX = 0;
        GameState->PlayerPos.TileMapY = 0;
        GameState->PlayerPos.TileX = 3;
        GameState->PlayerPos.TileY = 3;
        GameState->PlayerPos.X = 0.5f;
        GameState->PlayerPos.Y = 0.5f;
        Memory->IsInitialized = true;
    }

    tile_map *TileMap = GetTileMap(&World, GameState->PlayerPos.TileMapX, GameState->PlayerPos.TileMapY);
    Assert(TileMap);

    for (int ControllerIndex = 0; ControllerIndex < ArrayCount(Input->Controllers); ControllerIndex++)
    {
        game_controller_input *Controller = &Input->Controllers[ControllerIndex];
        if (Controller->IsAnalog)
        {
            // NOTE(george): Use analog movement tuning
        }
        else
        {
            // NOTE(george): Use digital movement tuning
            real32 dPlayerX = 0.0f; // pixels/second
            real32 dPlayerY = 0.0f; // pixels/second

            if (Controller->MoveUp.EndedDown)
            {
                dPlayerY = +1.0f;
            }
            if (Controller->MoveDown.EndedDown)
            {
                dPlayerY = -1.0f;
            }
            if (Controller->MoveLeft.EndedDown)
            {
                dPlayerX = -1.0f;
            }
            if (Controller->MoveRight.EndedDown)
            {
                dPlayerX = 1.0f;
            }
            dPlayerX *= 3.0f;
            dPlayerY *= 3.0f;

            world_position NewPos = GameState->PlayerPos;

            NewPos.X += dPlayerX * Input->dtForFrame;
            NewPos.Y += dPlayerY * Input->dtForFrame;
            NewPos = RecanonicalPosition(&World, NewPos);

            world_position NewPosLeft = NewPos;
            NewPosLeft.X -= PlayerWidth * -.5f;
            NewPosLeft = RecanonicalPosition(&World, NewPosLeft);

            world_position NewPosRight = NewPos;
            NewPosRight.X += PlayerWidth * .5f;
            NewPosRight = RecanonicalPosition(&World, NewPosRight);

            if (IsWorldPointEmpty(&World, NewPos) &&
                IsWorldPointEmpty(&World, NewPosLeft) &&
                IsWorldPointEmpty(&World, NewPosRight))
            {
                GameState->PlayerPos = NewPos;
                // world_position CanPos = GetCanonicalPosition(&World, PlayerPos);

                // GameState->PlayerTileMapX = CanPos.TileMapX;
                // GameState->PlayerTileMapY = CanPos.TileMapY;
                // GameState->PlayerX = World.UpperLeftX + World.TileSizeInPixels * CanPos.TileX + CanPos.X;
                // GameState->PlayerY = World.UpperLeftY + World.TileSizeInPixels * CanPos.TileY + CanPos.Y;
            }
        }
    }

    DrawRectangle(Buffer, 0.0f, 0.0f, (real32)Buffer->Width, (real32)Buffer->Height, 1.0f, 0.0f, 1.0f);

    for (int Row = 0; Row < World.CountY; Row++)
    {
        for (int Column = 0; Column < World.CountX; Column++)
        {
            uint32 TileID = GetTileValueUnchecked(&World, TileMap, Column, Row);
            real32 Gray = 0.5f;
            if (TileID == 1)
            {
                Gray = 1.0f;
            }

            if ((Column == GameState->PlayerPos.TileX) && (Row == GameState->PlayerPos.TileY))
            {
                Gray = 0.0f;
            }

            real32 MinX = World.LowerLeftX + (real32)Column * World.TileSizeInPixels;
            real32 MinY = World.LowerLeftY - ((real32)Row) * World.TileSizeInPixels;
            real32 MaxX = MinX + World.TileSizeInPixels;
            real32 MaxY = MinY - World.TileSizeInPixels;

            DrawRectangle(Buffer, MinX, MaxY, MaxX, MinY, Gray, Gray, Gray);
        }
    }

    real32 PlayerR = 1.0f;
    real32 PlayerG = 1.0f;
    real32 PlayerB = 0.0f;

    real32 PlayerLeft = World.LowerLeftX + World.TileSizeInPixels * GameState->PlayerPos.TileX + 
        World.MetersToPixels * GameState->PlayerPos.X - 0.5f * PlayerWidth * World.MetersToPixels;

    real32 PlayerTop = World.LowerLeftY - World.TileSizeInPixels * GameState->PlayerPos.TileY - 
        World.MetersToPixels * GameState->PlayerPos.Y - PlayerHeight * World.MetersToPixels;

    DrawRectangle(Buffer, PlayerLeft, PlayerTop, 
        PlayerLeft + World.MetersToPixels*PlayerWidth,
        PlayerTop + World.MetersToPixels*PlayerHeight,
        PlayerR, PlayerG, PlayerB);
}

extern "C" GAME_GET_SOUND_SAMPLES(GameGetSoundSamples)
{
    game_state *GameState = (game_state *)Memory->PermanentStorage;
    GameOutputSound(GameState, SoundBuffer, 400);
}

/*
internal void 
RenderWeirdGradient(game_offscreen_buffer *Buffer, int BlueOffset, int GreenOffset)
{
    uint8 *Row = (uint8 *)Buffer->Memory;
    for (int Y = 0; Y < Buffer->Height; Y++)
    {
        uint32 *Pixel = (uint32 *) Row;
        
        for (int X = 0; X < Buffer->Width; X++)
        {
            uint8 Blue = (uint8)(X + BlueOffset);
            uint8 Green = (uint8)(Y + GreenOffset);
            
            *Pixel++ = ((Green << 8) | Blue);   
        }
        
        Row += Buffer->Pitch;
    }
}
*/