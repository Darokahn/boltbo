enum controlType {
    startMove,
    stopMove,
    jump,
    startCrouch,
    stopCrouch,
};

typedef struct {
    enum controlType type;
    int length;
    uint8_t data[];
} control;

