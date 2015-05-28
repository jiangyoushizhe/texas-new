int hold[5], com[10];
#include <stdio.h>
#include <stdlib.h>
#include "socket.h"
#include "constant.h"
#include "conversion.h"
#include "opponent.h"
#include "Card.h"
#include<string.h>

#define AVGC 2
#ifdef TEST
#include <time.h>
#endif

extern ANAOPP opp[MAX_PLAYER_NUM];

struct player
{
	int pid, jetton, money;
}button, sblind, bblind, nor[10], my;//nor[0].pid is the number of other players.
//nor: other players.

struct player_in_game
{
	int pid, jetton, money, bet, action; //1:check  2:call  3:raise  4:all_in  5:fold  6:blind
}done[10]; //done[0].pid is the number of players.

struct player_rank
{
	int pid;
	int hold[5];
	int nut_hand;
}rank[10];//rank[0].pid is player number.

struct pot_win
{
	int pid, num;
}win[10];//win[0].pid is player number.

int plnum = 0; //player number
int pot = 0;

int get_msg(int fd)//1:seat_info  2:game_over  3:blind  4:hold  5:inquire  6:common cards  7:showdown  8:pot-win  9:notify
{
	char msg[25] = { 0 };
	get_word(msg, fd);
	if (strcmp(msg, "seat/") == 0){
		nor[0].pid = 0;
		plnum = 0;
		while (1){
			get_word(msg, fd);
			if (strcmp(msg, "/seat") == 0)return 1;
			if (strcmp(msg, "button:") == 0){
				int num = 0;
				get_word(msg, fd); change_to_num(msg, &num); button.pid = num;
				get_word(msg, fd); change_to_num(msg, &num); button.jetton = num;
				get_word(msg, fd); change_to_num(msg, &num); button.money = num;
				plnum++;
			}
			else if (strcmp(msg, "small") == 0){
				int num = 0;
				get_word(msg, fd);
				get_word(msg, fd); change_to_num(msg, &num); sblind.pid = num;
				get_word(msg, fd); change_to_num(msg, &num); sblind.jetton = num;
				get_word(msg, fd); change_to_num(msg, &num); sblind.money = num;
				plnum++;
			}
			else if (strcmp(msg, "big") == 0){
				int num = 0;
				get_word(msg, fd);
				get_word(msg, fd); change_to_num(msg, &num); bblind.pid = num;
				get_word(msg, fd); change_to_num(msg, &num); bblind.jetton = num;
				get_word(msg, fd); change_to_num(msg, &num); bblind.money = num;
				plnum++;
			}
			else{
				int num = 0;
				nor[0].pid++;
				int x = nor[0].pid;
				change_to_num(msg, &num); nor[x].pid = num;
				get_word(msg, fd); change_to_num(msg, &num); nor[x].jetton = num;
				get_word(msg, fd); change_to_num(msg, &num); nor[x].money = num;
				plnum++;
			}
		}
	}
	if (strcmp(msg, "game-over") == 0)return 2;
	if (strcmp(msg, "blind/") == 0){
		int num = 0;
		get_word(msg, fd); get_word(msg, fd); change_to_num(msg, &num); sblind.jetton -= num;
		while (1){
			get_word(msg, fd);
			if (strcmp(msg, "/blind") == 0)return 3;
			else{
				get_word(msg, fd); change_to_num(msg, &num); bblind.jetton -= num;
			}
		}
	}
	if (strcmp(msg, "hold/") == 0){
		char col[10];
		char poi[5];
		hold[0] = 0;
		while (1){
			get_word(msg, fd);
			if (strcmp(msg, "/hold") == 0)return 4;
			else{
				strcpy(col, msg);
				get_word(msg, fd); strcpy(poi, msg);
				hold[0]++;
				hold[hold[0]] = ctoi(col, poi);
			}
		}
	}
	if (strcmp(msg, "inquire/") == 0){
		done[0].pid = 0;
		while (1){
			int num = 0;
			get_word(msg, fd);
			if (strcmp(msg, "/inquire") == 0)return 5;
			else if (strcmp(msg, "total") == 0){
				get_word(msg, fd); get_word(msg, fd);
				change_to_num(msg, &num);
				pot = num;
			}
			else{
				done[0].pid++;
				int x = done[0].pid;
				change_to_num(msg, &num); done[x].pid = num;
				get_word(msg, fd); change_to_num(msg, &num); done[x].jetton = num;
				get_word(msg, fd); change_to_num(msg, &num); done[x].money = num;
				get_word(msg, fd); change_to_num(msg, &num); done[x].bet = num;
				get_word(msg, fd);
				if (strcmp(msg, "check") == 0)done[x].action = 1;
				if (strcmp(msg, "call") == 0)done[x].action = 2;
				if (strcmp(msg, "raise") == 0)done[x].action = 3;
				if (strcmp(msg, "all_in") == 0)done[x].action = 4;
				if (strcmp(msg, "fold") == 0)done[x].action = 5;
				if (strcmp(msg, "blind") == 0)done[x].action = 6;
			}
		}
	}
	if (strcmp(msg, "flop/") == 0 || strcmp(msg, "turn/") == 0 || strcmp(msg, "river/") == 0){
		char col[10];
		char poi[5];
		com[0] = 0;
		while (1){
			get_word(msg, fd);
			if (strcmp(msg, "/flop") == 0 || strcmp(msg, "/turn") == 0 || strcmp(msg, "/river") == 0)return 6;
			else{
				strcpy(col, msg);
				get_word(msg, fd); strcpy(poi, msg);
				com[0]++;
				com[com[0]] = ctoi(col, poi);
			}
		}
	}
	if (strcmp(msg, "showdown/") == 0){
		while (strcmp(msg, "/common") != 0)get_word(msg, fd);
		rank[0].pid = 0;
		while (1){
			get_word(msg, fd);
			if (strcmp(msg, "/showdown") == 0)return 7;
			else{
				int num, x;
				rank[0].pid++; x = rank[0].pid;
				get_word(msg, fd); change_to_num(msg, &num); rank[x].pid = num;
				int i;
				for (i = 1; i <= 2; i++){
					char col[10];
					char poi[5];
					get_word(msg, fd); strcpy(col, msg);
					get_word(msg, fd); strcpy(poi, msg);
					rank[x].hold[i] = ctoi(col, poi);
				}
				get_word(msg, fd);
				if (strcmp(msg, "HIGH_CARD") == 0)rank[x].nut_hand = 1;
				if (strcmp(msg, "ONE_PAIR") == 0)rank[x].nut_hand = 2;
				if (strcmp(msg, "TWO_PAIR") == 0)rank[x].nut_hand = 3;
				if (strcmp(msg, "THREE_OF_A_KIND") == 0)rank[x].nut_hand = 4;
				if (strcmp(msg, "STRAIGHT") == 0)rank[x].nut_hand = 5;
				if (strcmp(msg, "FLUSH") == 0)rank[x].nut_hand = 6;
				if (strcmp(msg, "FULL_HOUSE") == 0)rank[x].nut_hand = 7;
				if (strcmp(msg, "FOUR_OF_A_KIND") == 0)rank[x].nut_hand = 8;
				if (strcmp(msg, "STRAIGHT_FLUSH") == 0)rank[x].nut_hand = 9;
			}
		}
	}
	if (strcmp(msg, "pot-win/") == 0){
		win[0].pid = 0;
		while (1){
			get_word(msg, fd);
			if (strcmp(msg, "/pot-win") == 0)return 8;
			else{
				int num, x;
				win[0].pid++;
				x = win[0].pid;
				change_to_num(msg, &num); win[x].pid = num;
				get_word(msg, fd); change_to_num(msg, &num); win[x].num = num;
			}
		}
	}
	if (strcmp(msg, "notify/") == 0){
		done[0].pid = 0;
		while (1){
			int num = 0;
			get_word(msg, fd);
			if (strcmp(msg, "/notify") == 0)return 9;
			else if (strcmp(msg, "total") == 0){
				get_word(msg, fd); get_word(msg, fd);
				change_to_num(msg, &num);
				pot = num;
			}
			else{
				done[0].pid++;
				int x = done[0].pid;
				change_to_num(msg, &num); done[x].pid = num;
				get_word(msg, fd); change_to_num(msg, &num); done[x].jetton = num;
				get_word(msg, fd); change_to_num(msg, &num); done[x].money = num;
				get_word(msg, fd); change_to_num(msg, &num); done[x].bet = num;
				get_word(msg, fd);
				if (strcmp(msg, "check") == 0)done[x].action = 1;
				if (strcmp(msg, "call") == 0)done[x].action = 2;
				if (strcmp(msg, "raise") == 0)done[x].action = 3;
				if (strcmp(msg, "all_in") == 0)done[x].action = 4;
				if (strcmp(msg, "fold") == 0)done[x].action = 5;
				if (strcmp(msg, "blind") == 0)done[x].action = 6;
			}
		}
	}
}

int get_uplim(double winrate, int jet)
{
	int tmp = pot, f = 0;
	int i;
	for (i = 1; i <= done[0].pid; i++){
		if (done[i].pid == bblind.pid)f = 1;
		if (f == 1){
			int x = hash(done[i].pid);
			if (opp[x].maxbet[0])tmp += opp[x].maxbet[0];
			else tmp += 30;
		}
	}
	double para = 1.0;
	if ((double)jet / START_JETTON < 1.0)para = (double)jet / START_JETTON;
	int ans = ((double)tmp * winrate * para) / (1.0 - winrate * para) + 0.5;
	return ans;
	//(tmp + n) * winrate * jet/START_JETTON = n
	//tmp*r*para = - n*r*para + n
	//tmp*r*para / (1 - r*para) = n
}

int main(int argc, char* agrv[]) {
	/* connect to server */
	int fd; // socket id code
	char* serverName;
	char* hostName;
	unsigned short serverPort, hostPort, id;

#ifdef WRITE_IN_FILE
	FILE * fout = freopen("melog.txt", "w", stdout);
#endif

#ifdef TEST
	printf("%ld\n", time(0));
	Card hand_card[2], common_card[7];
#endif

	serverName = agrv[1];
	serverPort = atoi(agrv[2]);
	hostName = agrv[3];
	hostPort = atoi(agrv[4]);
	id = atoi(agrv[5]);

	fd = establishConnection(serverName, serverPort, hostName, hostPort);
	if (fd != -1) printf("Connection established.\n");
	else { printf("Connection failure. Program Abort.\n"); return 1; }

	reg(id, fd, "hdbdl need notify \n");

	my.pid = id;
	my.jetton = START_JETTON;
	my.money = START_MONEY;

	/* main round loop */
	int round, flag = 0, mybet = 0;
	for (round = 0; round < MAX_ROUND; round++) {
		mybet = 0;
		hold[0] = 0;
		com[0] = 0;
		while (1){
			//read
			int x = get_msg(fd);

			//quit
			if (x == GAME_OVER_MSG){
				disconnect(fd);
				return 0;
			}
	
			//pre action
			if (x == SEAT_MSG && round == 0){
				memset(opp, 0, sizeof(opp));
				int i = 1;
				opp[i].pid = button.pid;
				i++;
				opp[i].pid = sblind.pid;
				i++;
				if (plnum > 2)opp[i].pid = bblind.pid, i++;
				int j;
				for (j = 1; j <= nor[0].pid; j++){
					opp[i].pid = nor[j].pid;
					i++;
				}
			}
			if (x == INQUIRE_MSG || x == NOTIFY_MSG){
				int i;
				int stage = 1, f = 0;
				if (com[0] > 0)stage += com[0] - 2;
				for (i = 1; i <= done[0].pid; i++){
					if (done[i].pid == my.pid){
						my.jetton = done[i].jetton;
						my.money = done[i].money;
						mybet = done[i].bet;
					}
					if (done[i].pid == bblind.pid)f = 1;
					if (f == 0){
						int bet = 0;
						if (done[i].action == CHECK || done[i].action == ALLIN || done[i].action == FOLD);
						else bet = done[i].bet;
						updateData(done[i].pid, done[i].action, bet, done[i].jetton, done[i].money, stage, round);
					}
					if (f == 1){
						int bet = 0;
						if (done[i].action == CHECK || done[i].action == ALLIN || done[i].action == FOLD);
						else bet = done[i].bet;
						if (stage > 1)updateData(done[i].pid, done[i].action, bet, done[i].jetton, done[i].money, stage - 1, round);
						else updateData(done[i].pid, done[i].action, bet, done[i].jetton, done[i].money, stage, round);
					}
				}
#ifdef TEST
				double avrg = 0;
				rate R;
				if (com[0] >= 3) R = win_rate(hold + 1, com + 1, com[0], plnum); else R = make_pair(0.125, 0);
				double win = R.second;
				int maxbet = 0;
				for (int i = 0; i < 8; i++)
				{
					avrg += opp[i].avrgBet * (opp[i].jetton[round] + opp[i].money[round]);
					maxbet = max(maxbet, opp[i].lastbet[round]);
					printf("round: %d id: %d money: %d jetton: %d average bet: %lf last bet: %d\n", round, opp[i].pid, opp[i].money[round], opp[i].jetton[round], opp[i].avrgBet, opp[i].lastbet[round]);
				}
				printf("\n");
				avrg /= 28000;
				int mebet = (int)(avrg * win * AVGC);
				if (round < 10 || maxbet > mebet) action(FOLD, 0, fd); else action(RAISE, (int)(mebet - maxbet), fd);
#endif
			}
			if (x == SHOW_MSG){
				int i;
				for (i = 1; i <= rank[0].pid; i++){
					updateData(rank[i].pid, SHOW, rank[i].nut_hand, -1, -1, POT_WIN, round);
				}
			}
			if (x == POT_MSG){
				int i;
				for (i = 1; i <= win[0].pid; i++){
					//updateData(win[i].pid, POT, win[i].num, -1, -1, POT_WIN, round);
				}
				break;
			}
		}
	}

#ifdef WRITE_IN_FILE
	fclose(fout);
#endif
	return 0;
}
