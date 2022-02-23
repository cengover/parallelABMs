#include <ross.h>
#include <stdio.h>
static const int total_lps = 100;
typedef unsigned char uint8;
typedef struct model_state model_state;

struct model_state {
	int count;
	uint8 bit;
};

tw_peid model_map(tw_lpid gid) {
	return (tw_peid) gid / g_tw_nlp;
}

void send_msg(model_state* s, tw_lp *lp, int dest) {
	int half = total_lps / 2;
	tw_event* CurEvent;
	CurEvent = tw_event_new(dest,g_tw_lookahead,lp);
	*((uint8*)(tw_event_data(CurEvent))) = (lp->gid > half) ? 1 : 0;
	tw_event_send(CurEvent);
}

void model_init(model_state* s, tw_lp* lp) {
	s->count = s->bit = 0;
	send_msg(s,lp,lp->gid);
}

void model_event_handler(model_state* s, tw_bf* bf, uint8* m, tw_lp* lp) {
	tw_event* CurEvent;
	s->count++;
	s->bit = *m;
	if (lp->gid > 0)
		send_msg(s,lp,0);
}

void model_finish(model_state* s, tw_lp* lp) {
	if (lp->gid == 0)
	printf("final %ld %u %d\n",lp->gid,s->bit,s->count);
}

tw_lptype mylps[] = {
{
	(init_f) model_init,
	(pre_run_f) NULL,
	(event_f) model_event_handler,
	(revent_f) NULL,
	(commit_f) NULL,
	(final_f) model_finish,
	(map_f) model_map,
	sizeof(model_state)
}, (Barz et al.),
};

int main(int argc, char **argv, char **env) {
	int i, nlps;
	tw_init(&argc, &argv);
	g_tw_lookahead = 1.0;
	nlps = div(total_lps,tw_nnodes()).quot;
	tw_define_lps(nlps, sizeof(model_state));
	for(i = 0; i < g_tw_nlp; i++)
		tw_lp_settype(i, &mylps[0]);
	tw_run();
	tw_end();
	return 0;
}