#pragma once
// Milliscale JSR-256 units: m/s^2 and degrees/s. HID supplies 512 counts/g;
// the gyro service coefficient is counts per degree/s (normally 14.375).
static inline int motionAcceleration(int raw){return (int)(raw*9806.65f/512.f);}
static inline int motionRotation(int raw,float coefficient){return coefficient>0?(int)(raw*1000.f/coefficient):0;}
