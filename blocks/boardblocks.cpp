#include "boardblocks.h"

#include <iostream>
#include <sstream>
#include <cassert>
#include <cstdlib>
#include <list>
using std::cout;
using std::endl;

MoveBlocks::MoveBlocks(Token player,Color color) : Move(player), color(color) {}

void MoveBlocks::print() const {
	if (player==NOT_PLAYED) cout<<"block null move";
    std::cout<<"color ";
    switch (color) {
    case VIOLET:
        cout<<"violet";
        break;
    case ORANGE:
        cout<<"orange";
        break;
    case BLUE:
        cout<<"blue";
        break;
    case GREEN:
        cout<<"green";
        break;
    case YELLOW:
        cout<<"yellow";
        break;
    case RED:
        cout<<"red";
        break;
    case NONE:
        cout<<"none";
        break;
    }
    cout<<" for player "<<player;
}

Move *MoveBlocks::deepcopy() const {
	Move *copy=new MoveBlocks(player,color);
	return copy;
}

bool MoveBlocks::compare (const Move& abstract_move) const {
    const MoveBlocks &move=dynamic_cast<const MoveBlocks&>(abstract_move);
	return Move::compare(abstract_move) and color==move.color;
}

void BoardBlocks::TokenBlocks::print_char() const {
    //if (playable) {
    //    cout<<'x';
    //    return;
    //}

    std::string c;
    if (player==NOT_PLAYED) c = "█";
    else c="■";

    switch (color) {
    case VIOLET:
        c = "\e[35m"+c+"\e[0m";
        break;
    case BLUE:
        c = "\e[34m"+c+"\e[0m";
        break;
    case ORANGE:
        c = "\e[36m"+c+"\e[0m"; //This is actually Cyan
        break;
    case GREEN:
        c = "\e[32m"+c+"\e[0m";
        break;
    case YELLOW:
        c = "\e[33m"+c+"\e[0m";
        break;
    case RED:
        c = "\e[31m"+c+"\e[0m";
        break;
    case NONE:
        c = 'N';
        break;
    }

    cout<<c;
}

void BoardBlocks::TokenBlocks::print() const {
    cout<<"i="<<i<<" j="<<j<<" color="<<color<<" player="<<player;
}

BoardBlocks::BoardBlocks(Size width,Size height,bool init) : lastmove(NULL), width(width), height(height), size(width*height), p1score(1), p2score(1) {
	//allocate flat
	flat=new TokenBlocks[size];

    if (not init) return;

    assert(width%2==0 and height%2==0);
	for (Size row=0; row<height; row++) for (Size column=0; column<width/2; column++) {
        Color color = static_cast<Color>(rand()%6); //FIXME hardcoded color number
        {
            TokenBlocks &current = get_token(row,column);
            current.i        = row;
            current.j        = column;
            current.playable = false;
            current.player   = NOT_PLAYED;
            current.color    = color;
        }
        color = static_cast<Color>(5-color); //FIXME hardcoded color number
        {
            TokenBlocks &current = get_token(height-1-row,width-1-column);
            current.i        = height-1-row;
            current.j        = width-1-column;
            current.playable = false;
            current.player   = NOT_PLAYED;
            current.color    = color;
        }

    }

    get_token(height-1,0).player = PLAYER_1;
    get_token(0,width-1).player  = PLAYER_2;
    update_playable();
}

int BoardBlocks::get_p1score() const { return p1score; }
int BoardBlocks::get_p2score() const { return p2score; }
Token BoardBlocks::get_current_player() const {
    if (not lastmove) return PLAYER_1;
    else return other_player(lastmove->player);
}

void BoardBlocks::update_playable_token(TokenBlocks* current, const TokenBlocks* neighbor, const Color forbidden_color, Queue &queue) {
    if (current->player!=NOT_PLAYED or current->playable) return; //already played;
    if (neighbor->player!=NOT_PLAYED and current->color!=forbidden_color) queue.push(current);
    if (neighbor->player==NOT_PLAYED and current->color==neighbor->color) queue.push(current);
}

void BoardBlocks::update_playable() {
    Token player = PLAYER_1;
    Color forbidden_color  = NONE;
    if (lastmove) {
        player = other_player(lastmove->player);
        forbidden_color  = lastmove->color;
    }

    Queue queue;
    playable_colors.clear();

    for (int k=0; k<size; k++) {
        TokenBlocks *current = &flat[k];
        current->playable = false;
        if (current->player==player) queue.push(current);
    }

    while (not queue.empty()) {
        TokenBlocks *current = queue.top();
        queue.pop();

        assert(current->color!=forbidden_color);
        if (current->player==NOT_PLAYED) {
            current->playable = true;
            playable_colors.insert(current->color);
        }

        if (current->i>0)        update_playable_token(&get_token(current->i-1,current->j),current,forbidden_color,queue);
        if (current->i<height-1) update_playable_token(&get_token(current->i+1,current->j),current,forbidden_color,queue);
        if (current->j>0)        update_playable_token(&get_token(current->i,current->j-1),current,forbidden_color,queue);
        if (current->j<width-1)  update_playable_token(&get_token(current->i,current->j+1),current,forbidden_color,queue);
    }
}

BoardBlocks::TokenBlocks& BoardBlocks::get_token(Size i, Size j) {
    assert(i>=0 and i<height and j>=0 and j<width);
    return flat[j*height+i];
}

const BoardBlocks::TokenBlocks& BoardBlocks::get_const_token(Size i, Size j) const {
    assert(i>=0 and i<height and j>=0 and j<width);
    return flat[j*height+i];
}

const MoveBlocks* BoardBlocks::get_const_lastmove() const {
    return lastmove;
}

BoardBlocks::~BoardBlocks() {
	delete [] flat;
    if (lastmove) delete lastmove;
}

Board *BoardBlocks::deepcopy() const {
    BoardBlocks *copy=new BoardBlocks(width,height,false);

    //copy last move and played_count
    if (lastmove) copy->lastmove = static_cast<MoveBlocks*>(lastmove->deepcopy());

	//copy flat
    for (int k=0; k<size; k++) {
        TokenBlocks &dest = copy->flat[k];
        const TokenBlocks &orig = flat[k];
        dest.player = orig.player;
        dest.color  = orig.color;
        dest.i      = orig.i;
        dest.j      = orig.j;
        dest.playable = orig.playable;
    }

    copy->playable_colors.clear();
    for (Colors::const_iterator i=playable_colors.begin(); i!=playable_colors.end(); i++) {
        copy->playable_colors.insert(*i);
    }

    return copy;
}

Move *BoardBlocks::parse_move_string(Token player,const char *string) const {
    Color c;
    switch (string[0]) {
    case 'r':
        c = RED;
        break;
    case 'b':
        c = BLUE;
        break;
    case 'y':
        c = YELLOW;
        break;
    case 'g':
        c = GREEN;
        break;
    case 'o':
        c = ORANGE;
        break;
    case 'v':
        c = VIOLET;
        break;
    default:
        return NULL;
        break;
    }

	Move *move=new MoveBlocks(player,c);

	if (is_move_valid(*move)) return move;

	delete move;
	return NULL;
}

void BoardBlocks::print() const {
	std::cout<<"  ";
	for (Size column=0; column<width; column++) std::cout<<(column%10);
	std::cout<<std::endl;

	std::cout<<" +";
	for (Size column=0; column<width; column++) std::cout<<"-";
	std::cout<<"+"<<std::endl;

	for (Size row=0; row<height; row++) {
		std::cout<<(row%10)<<"|";
		for (Size column=0; column<width; column++) {
            const TokenBlocks &current = get_const_token(row,column);
            current.print_char();
		}
		std::cout<<"|"<<(row%10)<<std::endl;
	}

	std::cout<<" +";
	for (Size column=0; column<width; column++) std::cout<<"-";
	std::cout<<"+"<<std::endl;

	std::cout<<"  ";
	for (Size column=0; column<width; column++) std::cout<<(column%10);
	std::cout<<std::endl;

    cout<<endl;
    cout<<"p1 "<<p1score<<" ";
    cout<<"p2 "<<p2score<<endl;
}

bool BoardBlocks::is_move_valid(const Move &abstract_move) const {
	return is_move_valid(dynamic_cast<const MoveBlocks&>(abstract_move));
}

bool BoardBlocks::is_move_valid(const MoveBlocks &move) const {
    Token player = PLAYER_1;
    if (lastmove) player = other_player(lastmove->player);

    if (move.color==NONE or move.player!=player) return false;

    return (playable_colors.find(move.color)!=playable_colors.end());
}

Moves BoardBlocks::get_possible_moves(Token player) const {
	Moves moves;
    for (Colors::const_iterator i=playable_colors.begin(); i!=playable_colors.end(); i++) { moves.push_back(new MoveBlocks(player,*i)); }
	return moves;
}

void BoardBlocks::play_move(const Move &abstract_move) {
	const MoveBlocks &move=dynamic_cast<const MoveBlocks&>(abstract_move);

	assert(this->is_move_valid(move));

    for (int k=0; k<size; k++) {
        TokenBlocks &token = flat[k];

        if (token.playable and token.color==move.color) {
            token.player = move.player;
        }

        if (token.player==move.player) {
            token.color = move.color;
        }
    }

    if (lastmove) delete lastmove;
    lastmove = static_cast<MoveBlocks*>(move.deepcopy());
    //cout<<"END OF PLAY MOVE"<<endl;
    update_playable();

    if (lastmove->player==PLAYER_2) {
        p1score = 0;
        p2score = 0;

        for (int k=0; k<size; k++) {
            const TokenBlocks &current = flat[k];
            if (current.player==PLAYER_1) p1score++;
            if (current.player==PLAYER_2) p2score++;
        }
    }

}

bool BoardBlocks::play_random_move(Token player) {
    Moves possible_moves=get_possible_moves(player);
    if (possible_moves.empty()) return false;

    int selected=rand()/(RAND_MAX + 1.0) * possible_moves.size();
    Moves::const_iterator selected_iter=possible_moves.begin();
    while (selected>0) {
        selected--;
        selected_iter++;
    }
    play_move(**selected_iter);

    //play_move(*selected);
    //Move *selected=possible_moves[rand()%possible_moves.size()];
    //play_move(*selected);

    for (Moves::iterator iter=possible_moves.begin(); iter!=possible_moves.end(); iter++) delete *iter;

    return true;
}

Token BoardBlocks::check_for_win() const {
    int n = 0;

    for (int k=0; k<size; k++) {
        const TokenBlocks &current = flat[k];
        if (current.playable) n++;
    }

	if (n!=0) return NOT_PLAYED;

    if (p1score>p2score) return PLAYER_1;
    else return PLAYER_2;
}

