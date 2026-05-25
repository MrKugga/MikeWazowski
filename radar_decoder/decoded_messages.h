#pragma once
#include <cstdint>
#include <array>
#include <variant>
#include "../assignment/RadarTypes.h"

namespace RadarDecoded
{

// ── Common ────────────────────────────────────────────────────────────────

struct tPTPTimestamp
{
    uint32_t nSecHi;
    uint32_t nSecLo;
    uint32_t nNanoSec;
    RadarTypes::tPTPSync eSync;
};

// ── Single RDI detection (converted to float) ─────────────────────────────

struct tRDIDetection
{
    float    fRange;        // [m]
    float    fVrelRad;      // [m/s]
    float    fAzAng0;       // [rad] primary hypothesis
    float    fAzAng1;       // [rad] secondary hypothesis
    float    fElAng;        // [rad]
    float    fRCS0;         // [dBm^2]
    float    fRCS1;         // [dBm^2]
    float    fRangeVar;     // [m^2]
    float    fVrelRadVar;   // [(m/s)^2]
    float    fAzAngVar;     // [rad^2]
    float    fElAngVar;     // [rad^2]
    float    fSNR;          // [dB]
    uint8_t  nPdh0;         // false detection bit flags
};

// ── RDI messages ──────────────────────────────────────────────────────────

struct tRDIMessage
{
    uint8_t                  nSensorID;
    uint8_t                  nMessageCounter;
    tPTPTimestamp            sPTPTimestamp;
    uint32_t                 nTimeStamp;
    uint32_t                 nCycleCounter;
    RadarTypes::tSignalStatus eSignalStatus;
    float                    fVAmbig;        // [m/s]
    float                    fMaxRange;      // [m]
    uint16_t                 nNbOfDetections;
    uint16_t                 nArraySize;     // how many entries in aDetections
    std::array<tRDIDetection, 38> aDetections; // max 38 per message
};

// ── Single Object (converted to float) ───────────────────────────────────

struct tObjectDecoded
{
    uint8_t                      uObjId;
    float                        fDistX;         // [m]
    float                        fDistY;         // [m]
    float                        fVabsX;         // [m/s]
    float                        fVabsY;         // [m/s]
    float                        fAabsX;         // [m/s^2]
    float                        fAabsY;         // [m/s^2]
    float                        fDistXStd;      // [m]
    float                        fDistYStd;      // [m]
    float                        fVabsXStd;      // [m/s]
    float                        fVabsYStd;      // [m/s]
    float                        fAabsXStd;      // [m/s^2]
    float                        fAabsYStd;      // [m/s^2]
    float                        fLDeltaX[3];    // [m]
    float                        fLDeltaY[3];    // [m]
    RadarTypes::tShapeQualifier  eShapeQualifier;
    float                        fObjOrientation;// [rad]
    float                        fRCS;           // [dBm^2]
    uint8_t                      uProbOfExistence;
    uint16_t                     uLifeCycles;
    RadarTypes::tDynamicProperty eDynamicProperty;
    RadarTypes::tObjState        eObjState;
};

// ── Object message ────────────────────────────────────────────────────────

struct tObjectMessage
{
    uint8_t                   nSensorID;
    uint8_t                   nMessageCounter;
    tPTPTimestamp             sPTPTimestamp;
    uint32_t                  nTimeStamp;
    uint32_t                  nCycleCounter;
    RadarTypes::tSignalStatus eSignalStatus;
    float                     fEgoVx;          // [m/s]
    float                     fEgoYawRate;     // [rad/s]
    uint16_t                  nNbOfObjects;
    uint16_t                  nArraySize;      // 31
    std::array<tObjectDecoded, 31> aObjects;
};

// ── Sensor status ─────────────────────────────────────────────────────────

struct tSensorStatusDecoded
{
    uint8_t  nSensorID;
    float    fCurrentLongPos;       // [m]
    float    fCurrentLatPos;        // [m]
    float    fCurrentVertPos;       // [m]
    float    fCurrentLongPosCoG;    // [m]
    float    fCurrentYawAngle;      // [rad]
    float    fCurrentDamping;       // [dB]
    uint8_t  nDefective;
    uint8_t  nExtDisturbed;
    uint8_t  nComError;
    float    fAlnMisalignmentAzNear;// [rad]
    float    fAlnMisalignmentAzFar; // [rad]
    float    fAlnMisalignmentElev;  // [rad]
    uint8_t  nAlnStatus;
};

// ── Vehicle dynamics ──────────────────────────────────────────────────────

struct tVehicleDynamicsDecoded
{
    RadarTypes::tEgoLongDir eLongDir;
    float fLongVel;     // [m/s]
    float fYawRate;     // [rad/s]
    float fLongAccel;   // [m/s^2]
    float fLatAccel;    // [m/s^2]
};


// ── Variant ──────────────────────────────────────────────────────────────

/*
Variant + Visit are a storng combo as the compiler create a dispatch table at compile time.
Visit requires ONE callable (first argument) , that handles all. We use overload for this to
combine multiple lambdas into ONE 
*/

using DecodedMessage = std::variant<
    std::monostate,
    tRDIMessage,            // RDINEAR_0, RDINEAR_1, RDINEAR_2, RDIFAR_0, RDIFAR_1
    tObjectMessage,         // OBJECTS_0, OBJECTS_1
    tSensorStatusDecoded,   // SENSORSTATUS
    tVehicleDynamicsDecoded // VEHDYN
    // SENSORCONFIG  Tx only, not expected in recording
>;

/*
Create a struct called overloaded
It inherits from every type in Ts...
It brings all their operator() into scope

overloaded merges multiple lambdas into a single struct that has all their operator() overloads, 
so std::visit can call the right one based on what type the variant currently holds.
*/
template<typename... Ts>
struct overloaded : Ts... { using Ts::operator()...; };

/*
Deduction guide --> neede by the compiler. Maybe not needed for C++20??? 
*/
template<typename... Ts>
overloaded(Ts...) -> overloaded<Ts...>;

} // namespace RadarDecoded