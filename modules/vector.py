"""
Python modules for AoC
"""

class Vector:
    """
    A basic vector/position class
    """

    def __init__(self, r, c):
        self.row = r
        self.col = c

    def __add__(self, o):
        return Vector(self.row + o.row, self.col + o.col)

    def __sub__(self, o):
        return Vector(self.row - o.row, self.col - o.col)

    def __repr__(self):
        return f"({self.row},{self.col})"

    def __eq__(self, o):
        return self.row == o.row and self.col == o.col
