import math


def jump(t):
    return [
        0.0,
        math.cos(t * math.pi * 2.0 * 1.5) * 0.5 * min(t, 1.0),
        0.0,
        -math.cos(t * math.pi * 2.0 * 1.5) * 0.5 * min(t, 1.0),
    ]


def dance(t):
    return [
        0.0,
        math.sin(t * math.pi * 2.0 * 1.5) * 0.5 * min(t, 1.0),
        0.0,
        math.sin(t * math.pi * 2.0 * 1.5) * 0.5 * min(t, 1.0),
    ]


def balance(t):
    return [
        0.0,
        -min(t, 0.8) + math.sin(t * 4.2) * 0.2 * min(max(t - 0.5, 0.0), 1.0),
        0.0,
        min(t, 0.8) + math.sin(t * 4.2) * 0.2 * min(max(t - 0.5, 0.0), 1.0),
    ]


def turn_right(t):
    return [
        0.0,
        0.0,
        math.sin(t * math.pi * 2.0 * 1.5) * 0.5 * min(t, 1.0),
        math.cos(t * math.pi * 2.0 * 1.5) * 0.25 * min(t, 1.0),
    ]


def turn_left(t):
    return [
        math.sin(t * math.pi * 2.0 * 1.5) * 0.5 * min(t, 1.0),
        math.cos(t * math.pi * 2.0 * 1.5) * 0.25 * min(t, 1.0),
        0.0,
        0.0,
    ]


def walk(t):
    return [
        math.sin(t * math.pi * 2.0 * 1.5) * 0.5 * min(t, 1.0),
        math.cos(t * math.pi * 2.0 * 1.5) * 0.5 * min(t, 1.0),
        -math.sin(t * math.pi * 2.0 * 1.5) * 0.5 * min(t, 1.0),
        -math.cos(t * math.pi * 2.0 * 1.5) * 0.5 * min(t, 1.0),
    ]
