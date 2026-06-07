import math

def min_x_minus(p: int) -> float:
    return math.log2(1 - pow(2, -pow(2, -p)))

def min_x_plus(p: int) -> float:
    return math.log2(pow(2, pow(2, -p)) - 1)

precisions: [int] = [1, 3, 7, 9, 15, 23, 30, 40, 52]

for p in precisions:
    print(f"p^- = {p}: {min_x_minus(p)}, p^+ = {p}: {min_x_plus(p)}")
