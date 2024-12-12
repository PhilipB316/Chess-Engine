"""
This is going to be a really cool chess engine

Philip Brand
26-06-2024
"""

# import chess
# from chessboard import display          # type: ignore
import timeit
import numpy as np

LEGITIMATE_SQUARES = []

for i in range(1, 9):
    for j in range(1, 9):
        LEGITIMATE_SQUARES.append(10*i + j)

LEGITIMATE_SQUARES = np.array(LEGITIMATE_SQUARES)

fen = "r1bk3r/p2pBpNp/n4n2/1p1NP2P/6P1/3P4/P1P1K3/q5b1"
# board = chess.Board()

# game_board = display.start()

# display.update(fen, game_board)
# display.flip(game_board)



# while not display.check_for_quit():
#     time.sleep(1)


class Piece:

    def __init__(self, square, piece):
        """initialises useful characteristics of a piece"""

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
    

    def determine_all_possible_squares_for_straight_moves(self):
        """determines all possible squares for straight moves
        returns a list of lists containing possible squares"""

        row = self.row
        col = self.col
        square = self.square

        ###determines possible squares in each direction:
        number_of_free_squares_in_direction = 8 - row
        possible_squares_up = square + np.arange(1, number_of_free_squares_in_direction + 1, 1)
        number_of_free_squares_in_direction = row - 1
        possible_squares_down = square - np.arange(1, number_of_free_squares_in_direction + 1, 1)
        number_of_free_squares_in_direction = col - 1
        possible_squares_left = square - np.arange(10, 10*number_of_free_squares_in_direction + 1, 10)
        number_of_free_squares_in_direction = 8 - col
        possible_squares_right = square + np.arange(10, 10*number_of_free_squares_in_direction + 1, 10)

        all_possible_squares = [possible_squares_up,
                                possible_squares_down,
                                possible_squares_right,
                                possible_squares_left
                            ]
        return all_possible_squares
    
    
    def determine_all_possible_squares_for_diagonal_moves(self):
        """determines all possible squares for diagonal moves
        returns an list of lists containing possible squares"""

        row = self.row
        col = self.col
        square = self.square

        number_of_free_squares_in_direction = min(8 - row, col - 1)
        possible_squares_up_left = square - np.arange(9, 9*number_of_free_squares_in_direction + 1, 9)
        number_of_free_squares_in_direction = min(8 - row, 8 - col)
        possible_squares_up_right = square + np.arange(11, 11*number_of_free_squares_in_direction + 1, 11)
        number_of_free_squares_in_direction = min(row - 1, col - 1)
        possible_squares_down_left = square - np.arange(11, 11*number_of_free_squares_in_direction + 1, 11)
        number_of_free_squares_in_direction = min(row - 1, 8 - col)
        possible_squares_down_right = square + np.arange(9, 9*number_of_free_squares_in_direction + 1, 9)

        all_possible_squares = [possible_squares_up_left,
                               possible_squares_down_left,
                               possible_squares_up_right,
                               possible_squares_down_right
                               ]
        
        return all_possible_squares
    

    def determine_legitimate_squares_from_possible_squares(self, all_possible_squares, position):
        """finds all legitimate squares a piece can move to  from a list of lists containing possible moves
        returns a list of legitimate squares"""
        
        white = self.white
        square = self.square
        piece = self.piece

        legitimate_moves_list = []
        threatened_squares_list = []

        for possible_squares in all_possible_squares:
            not_run_out_of_squares = True
            for possible_square in possible_squares:
                piece_on_square = position[possible_square]
                threatened_squares_list.append(possible_square)
                if piece_on_square == 'x' and not_run_out_of_squares:
                    legitimate_moves_list.append(f'{piece}-{square}-{possible_square}')
                elif piece.lower() != 'k' and ((piece_on_square.isupper() and white) or (piece_on_square.islower() and not white)):
                    not_run_out_of_squares = False
                elif self.is_piece_of_opposite_colour_on_square(position, possible_square):
                    legitimate_moves_list.append(f'{piece}-{square}-{possible_square}')
                    if piece.lower() != 'k':
                        not_run_out_of_squares = False
        
        self.threatened_squares = threatened_squares_list
        return legitimate_moves_list



class Pawn(Piece):

    def legitimate_moves(self, position):

        """determines the legitimate moves that can be made"""
        piece = self.piece
        colour = self.colour
        square = self.square
        legitimate_moves_list = []
        threatened_squares_list = []

        potential_straight_move = square + np.array([colour])
        possible_straight_move = np.intersect1d(potential_straight_move, LEGITIMATE_SQUARES)

        potential_double_straight_move = square + np.array([2 * colour])
        possible_double_straight_move = np.intersect1d(potential_double_straight_move, LEGITIMATE_SQUARES)

        potential_diagonal_moves = square + np.array([- 9 * colour, 11 * colour])
        possible_diagonal_moves = np.intersect1d(potential_diagonal_moves, LEGITIMATE_SQUARES)


        ###possible capture moves:
        for possible_square in possible_diagonal_moves:
            if self.is_piece_of_opposite_colour_on_square(position, possible_square):
                legitimate_moves_list.append(f'{piece}-{square}-{possible_square}')
            threatened_squares_list.append(possible_square)

        ###moving straight:
        possible_square = possible_straight_move[0]

        if position[possible_square] == 'x':
            legitimate_moves_list.append(f'{piece}-{square}-{possible_square}')

            ###moving straight 2 moves on move one:
            possible_square = possible_double_straight_move[0]

            if position[possible_square] == 'x' and (square % 10 == 2 or square % 10 == 7):
                legitimate_moves_list.append(f'{piece}-{square}-{possible_square}')

        self.threatened_squares = threatened_squares_list
        return legitimate_moves_list



class Knight(Piece):

    def legitimate_moves(self, position):

        """determines the legitimate moves that can be made"""

        piece = self.piece
        square = self.square
        legitimate_moves_list = []
        threatened_squares_list = []
        ###determines the potential squares to move to:
        potential_squares = square + np.array([-21, -19, -12, -8, 8, 12, 19, 21])
        ###works out what potential squares are legitimate:
        possible_squares = np.intersect1d(potential_squares, LEGITIMATE_SQUARES)
        ###works out if the possible square can be moved to:
        for possible_square in possible_squares:
            if position[possible_square] == 'x' or self.is_piece_of_opposite_colour_on_square(position, possible_square):
                legitimate_moves_list.append(f'{piece}-{square}-{possible_square}')
            threatened_squares_list.append(possible_square)

        self.threatened_squares = threatened_squares_list
        return legitimate_moves_list



class Bishop(Piece):

    def legitimate_moves(self, position):
        
        """determines the legitimate diagonal moves that can be made"""
        
        ###determines possible squares in each direction:
        all_possible_squares  = self.determine_all_possible_squares_for_diagonal_moves()
        ###determines which possible squares are legitimate moves:
        legitimate_moves_list = self.determine_legitimate_squares_from_possible_squares(all_possible_squares, position)
        return legitimate_moves_list



class Rook(Piece):


    def legitimate_moves(self, position):
        """determines the legitimate straight moves that can be made"""

        all_possible_squares =  self.determine_all_possible_squares_for_straight_moves()
        legitimate_moves_list = self.determine_legitimate_squares_from_possible_squares(all_possible_squares, position)
        return legitimate_moves_list



class Queen(Piece):
    
    def legitimate_moves(self, position):
        """determines the legitimate queen moves that can be made"""

        all_possible_squares = self.determine_all_possible_squares_for_diagonal_moves()
        all_possible_squares += self.determine_all_possible_squares_for_straight_moves()
        legitimate_moves_list = self.determine_legitimate_squares_from_possible_squares(all_possible_squares, position)
        return legitimate_moves_list



class King(Piece):

    def determine_castling_squares(self, position, opposite_colour_threatened_squares_list, offset):
        """determines if the king can castle"""

        square = self.square
        
        castling_over_check = (square + offset) in opposite_colour_threatened_squares_list
        castling_into_check = (square + 2 * offset) in opposite_colour_threatened_squares_list
        castling_across_empty_squares = position[square + offset] == position[square + 2 * offset] == 'x'

        if not castling_over_check and not castling_into_check and castling_across_empty_squares:
            return square + 2 * offset

    
    def legitimate_moves(self, position, opposite_colour_threatened_squares_list, important_pieces_moved):
        """determines all the legitimate king moves that can be made"""

        square = self.square
        piece = self.piece

        potential_squares = square + np.array([-11, -10, -9, -1, 1, 9, 10, 11])
        possible_squares = np.intersect1d(potential_squares, LEGITIMATE_SQUARES)
        self.threatened_squares = possible_squares

        possible_squares_not_threatened = np.setdiff1d(possible_squares, opposite_colour_threatened_squares_list)
        legitimate_moves_list = self.determine_legitimate_squares_from_possible_squares([possible_squares_not_threatened], position)
        
        ###and castling moves:
        r1, king, r2 = [*important_pieces_moved]
        if king == 'f':
            if r1 == 'f':
                possible_square = self.determine_castling_squares(position, opposite_colour_threatened_squares_list, -10)
                if possible_square != None:
                    legitimate_moves_list.append(f'{piece}-{square}-{possible_square}')
            if r2 == 'f':
                possible_square = self.determine_castling_squares(position, opposite_colour_threatened_squares_list, +10)
                if possible_square != None:
                    legitimate_moves_list.append(f'{piece}-{square}-{possible_square}')

        return legitimate_moves_list



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



def collate_possible_moves(position, threatened_squares_of_opposite_colour, important_pieces_moved, whites_turn):
    """collates all possible moves in a given position given the squares being threatened of the opposite colour"""

    board = position.board
    threatened_squares = []
    moves_list = []

    if not whites_turn:

        for square, piece in position.board.items():

            if piece == 'p':
                pawn = Pawn(square, piece)
                moves_list += (pawn.legitimate_moves(board))
                threatened_squares += pawn.threatened_squares

            if piece == 'n':
                knight = Knight(square, piece)
                moves_list += (knight.legitimate_moves(board))
                threatened_squares += knight.threatened_squares

            if piece == 'b':
                bishop = Bishop(square, piece)
                moves_list += (bishop.legitimate_moves(board))
                threatened_squares += bishop.threatened_squares

            if piece == 'r':
                rook = Rook(square, piece)
                moves_list += (rook.legitimate_moves(board))
                threatened_squares += rook.threatened_squares

            if piece == 'q':
                queen = Queen(square, piece)
                moves_list += (queen.legitimate_moves(board))
                threatened_squares += queen.threatened_squares
                
            if piece == 'k':
                black_king_square = square
                black_king_piece = piece

        king = King(black_king_square, black_king_piece)
        moves_list += king.legitimate_moves(board, threatened_squares_of_opposite_colour, important_pieces_moved)

    if whites_turn:

        for square, piece in position.board.items():

            if piece == 'P':
                pawn = Pawn(square, piece)
                moves_list += (pawn.legitimate_moves(board))
                threatened_squares += pawn.threatened_squares

            if piece == 'N':
                knight = Knight(square, piece)
                moves_list += (knight.legitimate_moves(board))
                threatened_squares += knight.threatened_squares

            if piece == 'B':
                bishop = Bishop(square, piece)
                moves_list += (bishop.legitimate_moves(board))
                threatened_squares += bishop.threatened_squares

            if piece == 'R':
                rook = Rook(square, piece)
                moves_list += (rook.legitimate_moves(board))
                threatened_squares += rook.threatened_squares

            if piece == 'Q':
                queen = Queen(square, piece)
                moves_list += (queen.legitimate_moves(board))
                threatened_squares += queen.threatened_squares
                
            if piece == 'K':
                black_king_square = square
                black_king_piece = piece

        king = King(black_king_square, black_king_piece)
        moves_list += king.legitimate_moves(board, threatened_squares_of_opposite_colour, important_pieces_moved)

    return moves_list, threatened_squares


fen = "r2qkb1r/p1pp1p1p/bpn2n1B/3Pp1p1/8/2N1PNPB/PPPQ1P1P/R3K2R"


position = Position(fen)
position.fen_to_board()
position.board_to_fen()

for i in range(0, 10000):
    important_pieces_moved = 'fft'
    moves_list, threatened_squares = collate_possible_moves(position, [], important_pieces_moved, False)
    # print(moves_list)
    moves_list, threatened_squares = collate_possible_moves(position, threatened_squares, important_pieces_moved, True)
    # print(moves_list)

print(moves_list)