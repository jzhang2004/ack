#ifndef NORCSID
static char rcsidp5[] = "$Header$";
#endif

#include "parser.h"
#include "Lpars.h"

FILE *ofile;

outputnopt()
{
	openofile("dfa.c");
	outdfa();
	installofile();
	openofile("trans.c");
	outdotrans();
	installofile();
	openofile("incalls.r");
	outputincalls();
	installofile();
}

static char ofilename[80];
static char ofiletemp[80];

PRIVATE
openofile(filename)
	char *filename;
{
	char *strcpy(), *strcat();
	strcpy(ofilename,filename);
	strcpy(ofiletemp,filename);
	strcat(ofiletemp,".new");
	if((ofile=fopen(ofiletemp,"w"))==NULL) {
		fprintf(stderr,"Fatal Error: cannot open output file %s\n",ofiletemp);
		sys_stop(S_EXIT);
	}
}

PRIVATE
installofile()
{
	/*
	 * if contents of newly generated ofiletemp is different
	 * from that of ofilename then copy over old file else
	 * delete newly generated file
	 */
	register FILE	*f1, *f2;
	register int	c1, c2;
	fclose(ofile);
	if((f1 = fopen(ofiletemp,"r")) == NULL) {
		fprintf(stderr,"Fatal Error: cannont reopen file %s\n",ofiletemp);
		sys_stop(S_EXIT);
	}
	if((f2 = fopen(ofilename,"r")) == NULL) {
		fclose(f1);
		RENAME(ofiletemp,ofilename);
		return;
	}
	do {
		c1 = getc(f1);
		c2 = getc(f2);
	} while (c1 == c2 && c1 != EOF);
	fclose(f1);
	fclose(f2);
	if (c1 != c2) {
		RENAME(ofiletemp,ofilename);
	}
	else UNLINK(ofiletemp);
}

PRIVATE
UNLINK(x)
	char *x;
{
	/* Must remove the file "x" */
	unlink(x);	/* systemcall to remove file */
}

PRIVATE
RENAME(x,y)
	char *x, *y;
{
	/* Must move the file "x" to the file "y" */
	unlink(y);
	if(link(x,y)!=0) {
		fprintf(stderr,"Cannot link to %s",y);
		sys_stop(S_EXIT);
	}
	unlink(x);
}

PRIVATE
outdfa()
{
	int s;
	struct state *p;
	int nout, ncpy, ngto;
	int seenswitch;
	fprintf(ofile,"#include \"nopt.h\"\n");
	fprintf(ofile,"\n");
	fprintf(ofile,"int OO_maxreplacement = %d;\n", maxreplacement);
	fprintf(ofile,"int OO_state = 0;\n");
	fprintf(ofile,"\n");
	for(s=0;s<=higheststate;s++) {
		fprintf(ofile,"static dfa%d();\n",s);
	}
	fprintf(ofile,"\nint (*OO_fstate[])()=\n{\n");
	for(s=0;s<=higheststate;s++) {
		fprintf(ofile,"\tdfa%d,\n",s);
	}
	fprintf(ofile,"};\n\n");
	for(s=0;s<=higheststate;s++) {
		fprintf(ofile,"static dfa%d(opcode)\n",s);
		fprintf(ofile,"\tint opcode;\n");
		fprintf(ofile,"{\n");
		fprintf(ofile,"\t/* ");
		outmnems(patterns[s]);
		fprintf(ofile," */\n");
		seenswitch = 0;
		for(p=states[s];p!=(struct state *)NULL;p=p->next) {
			if(!seenswitch) {
				seenswitch++;
				fprintf(ofile,"\tswitch(opcode) {\n");
			}
			fprintf(ofile,"\tcase op_%s: ",p->op->id_text);
			if(actions[p->goto_state]==(struct action *)NULL)
				fprintf(ofile,"OO_state = %d; ",p->goto_state);
			else fprintf(ofile,"OO_%ddotrans(); ",p->goto_state);
			fprintf(ofile,"break;\n");
		}
		if(s==0) {
			if(!seenswitch) {
				seenswitch++;
				fprintf(ofile,"\tswitch(opcode) {\n");
			}
			fprintf(ofile,"\tdefault:\n");
			fprintf(ofile,"\t\tOO_flush();\n");
			fprintf(ofile,"\t\tbreak;\n");
		}
		else {
			if(seenswitch) fprintf(ofile,"\tdefault:\n");
			findfail(s,&nout,&ncpy,&ngto);
			fprintf(ofile,"\t\tOO_dodefault(%d,%d);\n",nout,ngto);
			if(actions[ngto]!=NULL)
				fprintf(ofile,"\t\tOO_%ddotrans();\n",ngto);
			if(seenswitch) fprintf(ofile,"\t\tbreak;\n");
		}
		if(seenswitch) fprintf(ofile,"\t}\n");
		fprintf(ofile,"}\n");
		fprintf(ofile,"\n");
	}
}

PRIVATE
outmnems(l)
	struct mnems l;
{
	int i;
	for(i=1;i<=l.m_len;i++) 
		fprintf(ofile,"%s ",l.m_elems[i-1]->op_code->id_text);
}

PRIVATE
outdotrans()
{
	int s;
	struct action *a;
	int seennontested;
	fprintf(ofile,"#include \"nopt.h\"\n\n");
	for(s=0;s<=higheststate;s++) {
		if(actions[s]!=(struct action *)NULL) {
			fprintf(ofile,"\nOO_%ddotrans() {\n",s);
			fprintf(ofile,"\tregister p_instr patt = OO_patternqueue;\n");
			fprintf(ofile,"\t/* ");
			outmnems(patterns[s]);
			fprintf(ofile," */\n");
			seennontested=0;
			for(a=actions[s];a!=(struct action *)NULL;a=a->next) {
				if(a->test!=(struct exp_node *)NULL) {
					fprintf(ofile,"\tif(");
					outexp(a->test,s);
					fprintf(ofile,") {\n");
					outoneaction(s,a);
					fprintf(ofile,"\t\treturn;\n");
					fprintf(ofile,"\t}\n");
				}
				else {
					if(seennontested) {
						fprintf(stderr,"parser: more than one untested action on state %d\n",s);
						nerrors++;
					}
					seennontested++;
					outoneaction(s,a);
				}
			}
			if(!seennontested)
				fprintf(ofile,"\tOO_state=%d;\n",s);
			fprintf(ofile,"}\n");
		}
		/*
		 * else fprintf(ofile,"\nOO_%ddotrans() {\n\tOO_state=%d;\n}\n",s,s);
		 */
	}
}

PRIVATE
outoneaction(s,a)
	int s;
	struct action *a;
{
	fprintf(ofile,"\t\t/* -> ");
	outmnems(a->replacement);
	fprintf(ofile," */\n");
	fprintf(ofile,"#ifdef STATS\n");
	fprintf(ofile,"\t\tif(OO_wrstats) fprintf(stderr,\"%d\\n\");\n",a->linenum);
	fprintf(ofile,"#endif\n");
	outrepl(s,a->replacement);
	findworst(a->replacement);
}

PRIVATE
outrepl(state,repl)
	int state;
	struct mnems repl;
{
	/*
	/* Contruct <repl>=r1 r2 ... rn and put on output queue.
	*/
	int n = repl.m_len;
	int i;
	for(i=1;i<=n;i++) {
		struct mnem_elem *ri = repl.m_elems[i-1];
		char *mnem = ri->op_code->id_text;
		switch(ri->op_code->id_argfmt) {
		case NOARG:
			fprintf(ofile,"\t\tEM_mkop(GETNXTREPL(),op_%s);\n",mnem);
			break;
		case CST:
			fprintf(ofile,"\t\tEM_mkcst(GETNXTREPL(),op_%s,",mnem);
			fprintf(ofile,"(arith)");
			outexp(ri->arg,state);
			fprintf(ofile,");\n");
			break;
		case CSTOPT:
			if(ri->arg) {
				fprintf(ofile,"\t\tEM_mkcst(GETNXTREPL(),op_%s,",mnem);
				fprintf(ofile,"(arith)");
				outexp(ri->arg,state);
			}
			else {
				fprintf(ofile,"\t\tEM_mknarg(GETNXTREPL(),op_%s);\n",mnem);
			}
			fprintf(ofile,");\n");
			break;
		case LAB:
			fprintf(ofile,"\t\tEM_mkilb(GETNXTREPL(),op_%s,",mnem);
			outexp(ri->arg,state);
			fprintf(ofile,");\n");
			break;
		case DEFILB:
			fprintf(ofile,"\t\tEM_mkdefilb(GETNXTREPL(),op_%s,",mnem);
			outexp(ri->arg,state);
			fprintf(ofile,");\n");
			break;
		case PNAM:
			fprintf(ofile,"\t\tEM_mkpro(GETNXTREPL(),op_%s,",mnem);
			outexp(ri->arg,state);
			fprintf(ofile,");\n");
			break;
		case EXT:
			fprintf(ofile,"\t\tOO_mkext(GETNXTREPL(),op_%s,",mnem);
			outexp(ri->arg,state);
			fprintf(ofile,");\n");
			break;
		}
	}
}

PRIVATE
outexp(e,state)
	struct exp_node *e;
	int state;
{
	switch(e->node_type) {
	case LOGAND:
	case LOGOR:
	case BITAND:
	case BITOR:
	case XOR:
	case MINUS:
	case PLUS:
	case TIMES:
	case DIV:
	case MOD:
	case EQ:
	case NE:
	case LT:
	case LE:
	case GT:
	case GE:
	case LSHIFT:
	case RSHIFT:
		fprintf(ofile,"(");
		outexp(e->exp_left,state);
		outop(e->node_type);
		outexp(e->exp_right,state);
		fprintf(ofile,")");
		break;
	case NOT:
	case COMP:
	case UPLUS:
	case UMINUS:
		fprintf(ofile,"(");
		outop(e->node_type);
		outexp(e->exp_left,state);
		fprintf(ofile,")");
		break;
	case DEFINED:
		fprintf(ofile,"DEFINED(patt[%d])",e->leaf_val-1);
		break;
	case UNDEFINED:
		fprintf(ofile,"!DEFINED(patt[%d])",e->leaf_val-1);
		break;
	case COMMA:
		outext(e->exp_left);
		fprintf(ofile,",");
		fprintf(ofile,"(arith)");
		outexp(e->exp_right,state);
		break;
	case SAMESIGN:
	case SFIT:
	case UFIT:
	case ROTATE:
		outop(e->node_type);
		fprintf(ofile,"(arith)");
		outexp(e->exp_left,state);
		fprintf(ofile,",");
		fprintf(ofile,"(arith)");
		outexp(e->exp_right,state);
		fprintf(ofile,")");
		break;
	case SAMEEXT:
	case SAMENAM:
		outop(e->node_type);
		outext(e->exp_left);
		fprintf(ofile,",");
		outext(e->exp_right);
		fprintf(ofile,")");
		break;
	case PATARG:
		switch(patterns[state].m_elems[e->leaf_val-1]->op_code->id_argfmt) {
		case NOARG:
			fprintf(stderr,"error: mnem %d has no argument\n",e->leaf_val);
			nerrors++;
			break;
		case CST:
		case CSTOPT:
			fprintf(ofile,"CST(patt[%d])",e->leaf_val-1);
			break;
		case LAB:
			fprintf(ofile,"LAB(patt[%d])",e->leaf_val-1);
			break;
		case DEFILB:
			fprintf(ofile,"DEFILB(patt[%d])",e->leaf_val-1);
			break;
		case PNAM:
			fprintf(ofile,"PNAM(patt[%d])",e->leaf_val-1);
			break;
		case EXT:
			fprintf(ofile,"OO_offset(patt+%d)",e->leaf_val-1);
			break;
		}
		break;
	case PSIZE:
		fprintf(ofile,"OO_PSIZE"); break;
	case WSIZE:
		fprintf(ofile,"OO_WSIZE"); break;
	case INT:
		fprintf(ofile,"%d",e->leaf_val); break;
	}
}

PRIVATE
outext(e)
	struct exp_node *e;
{
	if(e->node_type!=PATARG) {
		fprintf(stderr,"Internal error in outext of parser\n");
		nerrors++;
	}
	fprintf(ofile,"patt+%d",e->leaf_val-1);
}

PRIVATE
outop(op)
	int op;
{
	switch(op) {
	case LOGAND:	fprintf(ofile,"&&");	break;
	case LOGOR:	fprintf(ofile,"||");	break;
	case BITAND:	fprintf(ofile,"&");	break;
	case BITOR:	fprintf(ofile,"|");	break;
	case XOR:	fprintf(ofile,"^");	break;
	case MINUS:	fprintf(ofile,"-");	break;
	case PLUS:	fprintf(ofile,"+");	break;
	case TIMES:	fprintf(ofile,"*");	break;
	case DIV:	fprintf(ofile,"/");	break;
	case MOD:	fprintf(ofile,"%%");	break;
	case EQ:	fprintf(ofile,"==");	break;
	case NE:	fprintf(ofile,"!=");	break;
	case LT:	fprintf(ofile,"<");	break;
	case LE:	fprintf(ofile,"<=");	break;
	case GT:	fprintf(ofile,">");	break;
	case GE:	fprintf(ofile,">=");	break;
	case LSHIFT:	fprintf(ofile,"<<");	break;
	case RSHIFT:	fprintf(ofile,">>");	break;
	case NOT:	fprintf(ofile,"!");	break;
	case COMP:	fprintf(ofile,"~");	break;
	case UPLUS:	fprintf(ofile,"+");	break;
	case UMINUS:	fprintf(ofile,"-");	break;
	case SAMESIGN:	fprintf(ofile,"OO_signsame(");	break;
	case SFIT:	fprintf(ofile,"OO_sfit(");	break;
	case UFIT:	fprintf(ofile,"OO_ufit(");	break;
	case ROTATE:	fprintf(ofile,"OO_rotate(");	break;
	case SAMEEXT:	fprintf(ofile,"OO_extsame(");	break;
	case SAMENAM:	fprintf(ofile,"OO_namsame(");	break;
	}
}
