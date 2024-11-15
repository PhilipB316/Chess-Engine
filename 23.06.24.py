"""
This is going to be a really cool chess engine

Philip Brand
26-06-2024
"""

import chess
from chessboard import display          # type: ignore
import timeit
import numpy as np

POSSIBLE_SQUARES = []

for i in range(1, 9):
    for j in range(1, 9):
        POSSIBLE_SQUARES.append(10*i + j)

POSSIBLE_SQUARES = np.array(POSSIBLE_SQUARES)

fen = "r1bk3r/p2pBpNp/n4n2/1p1NP2P/6P1/3P4/P1P1K3/q5b1"
board = chess.Board()

# game_board = display.start()

# display.update(fen, game_board)
# display.flip(game_board)



# while not display.check_for_quit():
#     time.sleep(1)


class Piece:

    def __init__(self, square, piece):

        self.piece = piece
        self.square = square
        self.white = piece.isupper()
        if piece.islower():
            self.colour = -1
        else:
            self.colour = 1
        
        self.row = row = square % 10
        self.col = int((square - row) / 10)

    def is_piece_of_opposite_colour_on_square(self, position, square):
        
        """determines if a piece of the opposite colour is on a square"""

        piece = position[square]
        if piece != 'x':
            return (piece.isupper() and not self.white) or (piece.islower() and self.white)
        return False
    

class Pawn(Piece):

    def possible_moves(self, position):

        """determines the possible moves that can be made"""
        piece = self.piece
        colour = self.colour
        square = self.square
        possible_moves_list = []

        potential_straight_move = square + np.array([colour])
        possible_straight_move = np.intersect1d(potential_straight_move, POSSIBLE_SQUARES)

        potential_double_straight_move = square + np.array([2 * colour])
        possible_double_straight_move = np.intersect1d(potential_double_straight_move, POSSIBLE_SQUARES)

        potential_diagonal_moves = square + np.array([- 9 * colour, 11 * colour])
        possible_diagonal_moves = np.intersect1d(potential_diagonal_moves, POSSIBLE_SQUARES)


        ###possible capture moves:
        for possible_square in possible_diagonal_moves:
            if self.is_piece_of_opposite_colour_on_square(position, possible_square):
                possible_moves_list.append(f'{piece}-{square}-{possible_square}')

        ###moving straight:
        possible_square = possible_straight_move[0]

        if position[possible_square] == 'x':
            possible_moves_list.append(f'{piece}-{square}-{possible_square}')

            ###moving straight 2 moves on move one:
            possible_square = possible_double_straight_move[0]

            if position[possible_square] == 'x' and (square % 10 == 2 or square % 10 == 7):
                possible_moves_list.append(f'{piece}-{square}-{possible_square}')

        return possible_moves_list


class Knight(Piece):

    def possible_moves(self, position):

        """determines the possible moves that can be made"""

        piece = self.piece
        square = self.square
        possible_moves_list = []
        ###determines the potential squares to move to:
        potential_squares = square + np.array([-21, -19, -12, -8, 8, 12, 19, 21])
        ###works out what potential squares are legitimate:
        possible_squares = np.intersect1d(potential_squares, POSSIBLE_SQUARES)
        ###works out if the possible square can be moved to:
        for possible_square in possible_squares:
            if position[possible_square] == 'x' or self.is_piece_of_opposite_colour_on_square(position, possible_square):
                possible_moves_list.append(f'{piece}-{square}-{possible_square}')

        return possible_moves_list


class Bishop(Piece):

    def possible_moves__diagonal(self, position):
        
        """determines the possible diagonal moves that can be made"""
        
        piece = self.piece
        square = self.square
        white = self.white
        row = self.row
        col = self.col

        possible_moves_list = []

        ###determines possible squares in each direction:
        number_of_free_squares_in_direction = min(8 - row, col - 1)
        possible_squares_up_left = np.intersect1d((square - np.arange(9, 9*number_of_free_squares_in_direction + 1, 9)), POSSIBLE_SQUARES)
        number_of_free_squares_in_direction = min(8 - row, 8 - col)
        possible_squares_up_right = np.intersect1d((square + np.arange(11, 11*number_of_free_squares_in_direction + 1, 11)), POSSIBLE_SQUARES)
        number_of_free_squares_in_direction = min(row - 1, col - 1)
        possible_squares_down_left = np.intersect1d((square - np.arange(11, 11*number_of_free_squares_in_direction + 1, 11)), POSSIBLE_SQUARES)
        number_of_free_squares_in_direction = min(row - 1, 8 - col)
        possible_squares_down_right = np.intersect1d((square + np.arange(9, 9*number_of_free_squares_in_direction + 1, 9)), POSSIBLE_SQUARES)

        all_possible_squares = [np.flip(possible_squares_up_left),
                               np.flip(possible_squares_down_left),
                               possible_squares_up_right,
                               possible_squares_down_right
                               ]
        
        for possible_squares in all_possible_squares:
            not_run_out_of_squares = True
            for possible_square in possible_squares:
                piece_on_square = position[possible_square]
                if piece_on_square == 'x' and not_run_out_of_squares:
                    possible_moves_list.append(f'{piece}-{square}-{possible_square}')
                elif (piece_on_square.isupper() and white) or (piece_on_square.islower() and not white):
                    not_run_out_of_squares = False
                elif self.is_piece_of_opposite_colour_on_square(position, possible_square):
                    possible_moves_list.append(f'{piece}-{square}-{possible_square}')
                    not_run_out_of_squares = False

        return possible_moves_list


class Rook(Piece):

    """determines the possible straight moves that can be made"""

    def possible_moves__straight(self, position):

        piece = self.piece
        square = self.square
        white = self.white
        row = self.row
        col = self.col

        def determine_all_possible_squares_for_straight_moves(position):

            ###determines possible squares in each direction:
            number_of_free_squares_in_direction = 8 - row
            possible_squares_up = np.intersect1d((square + np.arange(1, number_of_free_squares_in_direction + 1, 1)), POSSIBLE_SQUARES) 
            number_of_free_squares_in_direction = row - 1
            possible_squares_down = np.intersect1d((square - np.arange(1, number_of_free_squares_in_direction + 1, 1)), POSSIBLE_SQUARES)
            number_of_free_squares_in_direction = col - 1
            possible_squares_left = np.intersect1d((square - np.arange(10, 10*number_of_free_squares_in_direction + 1, 10)), POSSIBLE_SQUARES)
            number_of_free_squares_in_direction = 8 - col
            possible_squares_right = np.intersect1d((square + np.arange(10, 10*number_of_free_squares_in_direction + 1, 10)), POSSIBLE_SQUARES)

            all_possible_squares = [possible_squares_up,
                                    np.flip(possible_squares_down),
                                    possible_squares_right,
                                    np.flip(possible_squares_left)
                                ]

            return all_possible_squares
        
        possible_moves_list = []

        all_possible_squares =  determine_all_possible_squares_for_straight_moves(position)

        for possible_squares in all_possible_squares:
            not_run_out_of_squares = True
            for possible_square in possible_squares:
                piece_on_square = position[possible_square]
                if piece_on_square == 'x' and not_run_out_of_squares:
                    possible_moves_list.append(f'{piece}-{square}-{possible_square}')
                elif (piece_on_square.isupper() and white) or (piece_on_square.islower() and not white):
                    not_run_out_of_squares = False
                elif self.is_piece_of_opposite_colour_on_square(position, possible_square):
                    possible_moves_list.append(f'{piece}-{square}-{possible_square}')
                    not_run_out_of_squares = False

        return possible_moves_list







class Position:

    def __init__(self, fen):

        self.fen = fen

    def fen_to_board(self):

        """takes a position in fen and converts it to a dictionary mapping each square
        encoded as 'col' + 'row' to the piece
        """
        fen = self.fen
        rows_of_pieces = fen.split('/')
        board = {}
        for row in range(1, 9):
            characters = rows_of_pieces[row - 1]
            col = 1

            for character in characters:

                if character.isalpha():
                    square = int((str(col) + str(9 - row)))
                    board[square] = character
                    col += 1

                if character.isdigit():
                    character = int(character)
                    while character >= 1:
                        square = int((str(col) + str(9 - row)))
                        board[square] = 'x'
                        character -= 1
                        col += 1
        self.board = board
        # print(board)

    
    def board_to_fen(self):

        """takes a dictionary of position and converts it back to a fen"""

        board = self.board
        fen = ''
        i = 0
        was_just_empty = False
        # board = dict(sorted(board.items()))
        for square, piece in (board.items()):
            row = square % 10
            col = int((square - row) / 10)

            if piece != 'x' and not was_just_empty:
                fen += piece
            elif piece == 'x':
                i += 1
                was_just_empty = True
            elif piece != 'x' and was_just_empty:
                fen += f'{i}'
                fen += piece
                i = 0
                was_just_empty = False
            if col == 8 and row != 1:
                if piece == 'x':
                    fen += f'{i}'
                    was_just_empty = False
                    i = 0
                fen += '/'
            elif col == 8 and row == 1 and was_just_empty:
                fen += f'{i}'
        

        self.fen = fen






# fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR"
# fen = "8/8/8/8/8/2B5/8/8 w - - 0 1"
position = Position(fen)
position.fen_to_board()
position.board_to_fen()


moves_list = []
for square, piece in position.board.items():

    if piece.lower() == 'p':
        pawn = Pawn(square, piece)
        moves_list += (pawn.possible_moves(position.board))

    if piece.lower() == 'n':
        knight = Knight(square, piece)
        moves_list += (knight.possible_moves(position.board))

    if piece.lower() == 'b':
        bishop = Bishop(square, piece)
        moves_list += (bishop.possible_moves__diagonal(position.board))

    if piece.lower() == 'r':
        rook = Rook(square, piece)
        moves_list += (rook.possible_moves__straight(position.board))

print(sorted(moves_list))
print(len(moves_list))