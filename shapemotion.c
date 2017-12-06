/** \file shapemotion.c
 *  \brief This is a simple shape motion demo.
 *  This demo creates two layers containing shapes.
 *  One layer contains a rectangle and the other a circle.
 *  While the CPU is running the green LED is on, and
 *  when the screen does not need to be redrawn the CPU
 *  is turned off along with the green LED.
 */  
#include <msp430.h>
#include <libTimer.h>
#include <lcdutils.h>
#include <lcddraw.h>
#include <p2switches.h>
#include <shape.h>
#include <abCircle.h>
#include "buzzer.h"
#include "states.h"

#define GREEN_LED BIT6

int lscore = 0;
int rscore = 0;
char score[3];


AbRect rect10 = {abRectGetBounds, abRectCheck, {3,20}}; /**< 10x10 rectangle */


AbRectOutline fieldOutline = {	/* playing field */
		abRectOutlineGetBounds, abRectOutlineCheck,   
		{(screenWidth/2)-.5, (screenHeight/2)-.5}
};

Layer fieldLayer = {		//playing field as a layer 
		(AbShape *) &fieldOutline,
		{screenWidth/2, screenHeight/2},//< center
		{0,0}, {0,0},				    // last & next pos 
		COLOR_BLACK,
		0
};

Layer layer2 = {
		(AbShape *)&rect10,
		{(screenWidth/10)-4, (screenHeight/2)}, /**< bit below & right of center */
		{0,0}, {0,0},				    /* last & next pos */
		COLOR_RED,
		&fieldLayer
};

Layer layer1 = {		//< Layer with a red square
		(AbShape *)&rect10,
		{screenWidth-7, (screenHeight/2)}, //< center
		{0,0}, {0,0},				    // last & next pos
		COLOR_PURPLE,
		&layer2,
};

Layer layer0 = {		//< Layer with an orange circle 
		(AbShape *)&circle4,
		{(screenWidth/2), (screenHeight/2)}, //< bit below & right of center 
		{0,0}, {0,0},				    //last & next pos 
		COLOR_WHITE,
		&layer1,
};

/** Moving Layer
 *  Linked list of layer references
 *  Velocity represents one iteration of change (direction & magnitude)
 */
typedef struct MovLayer_s {
	Layer *layer;
	Vec2 velocity;
	struct MovLayer_s *next;
} MovLayer;

/* initial value of {0,0} will be overwritten */

MovLayer ml2 = { &layer2, {0,0}, 0};//left
MovLayer ml1 = { &layer1, {0,0}, 0};//right
MovLayer ml0 = { &layer0, {2,3}, 0};//ball


void movLayerDraw(MovLayer *movLayers, Layer *layers)
{
	int row, col;
	MovLayer *movLayer;

	and_sr(~8);			/**< disable interrupts (GIE off) */
	for (movLayer = movLayers; movLayer; movLayer = movLayer->next) { /* for each moving layer */
		Layer *l = movLayer->layer;
		l->posLast = l->pos;
		l->pos = l->posNext;
	}
	or_sr(8);			/**< disable interrupts (GIE on) */


	for (movLayer = movLayers; movLayer; movLayer = movLayer->next) { /* for each moving layer */
		Region bounds;
		layerGetBounds(movLayer->layer, &bounds);
		lcd_setArea(bounds.topLeft.axes[0], bounds.topLeft.axes[1], 
				bounds.botRight.axes[0], bounds.botRight.axes[1]);
		for (row = bounds.topLeft.axes[1]; row <= bounds.botRight.axes[1]; row++) {
			for (col = bounds.topLeft.axes[0]; col <= bounds.botRight.axes[0]; col++) {
				Vec2 pixelPos = {col, row};
				u_int color = bgColor;
				Layer *probeLayer;
				for (probeLayer = layers; probeLayer; 
						probeLayer = probeLayer->next) { /* probe all layers, in order */
					if (abShapeCheck(probeLayer->abShape, &probeLayer->pos, &pixelPos)) {
						color = probeLayer->color;
						break; 
					} /* if probe check */
				} // for checking all layers at col, row
				lcd_writeColor(color); 
			} // for col
		} // for row
	} // for moving layer being updated
}	  



//Region fence = {{10,30}, {SHORT_EDGE_PIXELS-10, LONG_EDGE_PIXELS-10}}; /**< Create a fence region */

/** Advances a moving shape within a fence
 *  
 *  \param ml The moving shape to be advanced
 *  \param fence The region which will serve as a boundary for ml
 */
void mlAdvance(MovLayer *ml, MovLayer *p1, MovLayer *p2, Region *fence)
{//                      ball         paddle1        paddle2  

	score[1] = '|';
	Vec2 newPos;
	u_char axis;
	Region shapeBoundary;
    //paddle1
    Region inner;
    Vec2 newPosP1;
	u_char axisP1;
	Region shapeBoundaryP1;
    //paddle2
    Vec2 newPosP2;
	u_char axisP2;
	Region shapeBoundaryP2;
    
    inner.topLeft.axes[0] = fence->topLeft.axes[0] + 7;
    inner.topLeft.axes[1] = fence->topLeft.axes[1];
    inner.botRight.axes[0] = fence->botRight.axes[0] - 7;
    inner.botRight.axes[1] = fence->botRight.axes[1];
    
	for (; ml; ml = ml->next) {
        buzzer_set_period(0);
		vec2Add(&newPos, &ml->layer->posNext, &ml->velocity);
		abShapeGetBounds(ml->layer->abShape, &newPos, &shapeBoundary);
        
		vec2Add(&newPosP1, &p1->layer->posNext, &p1->velocity);
		abShapeGetBounds(p1->layer->abShape, &newPosP1, &shapeBoundaryP1);
        
        vec2Add(&newPosP2, &p2->layer->posNext, &p2->velocity);
		abShapeGetBounds(p2->layer->abShape, &newPosP2, &shapeBoundaryP2);
        for (axis = 0; axis < 2; axis ++){
            //added for p1
            if((shapeBoundary.topLeft.axes[axis] < inner.topLeft.axes[axis]) || (shapeBoundary.botRight.axes[axis] > inner.botRight.axes[axis])){
                if(shapeBoundary.topLeft.axes[1] > shapeBoundaryP1.topLeft.axes[1] && shapeBoundary.botRight.axes[1] < shapeBoundaryP1.botRight.axes[1] && shapeBoundary.topLeft.axes[0] > (screenWidth/2)){
                     int velocity = ml->velocity.axes[0] = -ml->velocity.axes[0];
                    buzzer_set_period(5000);
                    newPos.axes[0] += (2*velocity);
                    break;
                }
            }//added for p2
            if((shapeBoundary.topLeft.axes[axis] < inner.topLeft.axes[axis]) || (shapeBoundary.botRight.axes[axis] > inner.botRight.axes[axis])){
                if(shapeBoundary.topLeft.axes[1] > shapeBoundaryP2.topLeft.axes[1] && shapeBoundary.botRight.axes[1] < shapeBoundaryP2.botRight.axes[1] && shapeBoundary.topLeft.axes[0] < (screenWidth/2)){
                     int velocity = ml->velocity.axes[0] = -ml->velocity.axes[0];
                    buzzer_set_period(4000);
                    newPos.axes[0] += (2*velocity);
                    break;
                }
            }//added
            if ((shapeBoundary.topLeft.axes[axis] < fence->topLeft.axes[axis]) || (shapeBoundary.botRight.axes[axis] > fence->botRight.axes[axis])) {
				int velocity = ml->velocity.axes[axis] = -ml->velocity.axes[axis];
                buzzer_set_period(2500);
				newPos.axes[axis] += (2*velocity);
			} /**< for axis */
			//if((ml->layer->posNext.axes[1] >= p1->layer->posNext.axes[1]) || (ml->layer->posNext.axes[0] <=  p1->layer->posNext.axes[0] + 20 && ml->layer->posNext.axes[0] >= orang  e->layer->posNext.axes[0] - 20)) {
			/*if(ml->layer->posNext.axes[0] == p1->layer->posNext.axes[0] && ml->layer->posNext.axes[1] == p1->layer->posNext.axes[1]){
                int velocity = ml->velocity.axes[axis] = -ml->velocity.axes[axis];
                int x2 = ml->velocity->axes[0];
                int y2 = ml->velocity->axes[1];
                x2 *= -1;
                if(y2 < 0){
                    y2 -= (ml->velocity->axes[1]);
                }
                //ml->velocity.axes[0] += 1;
                newPos.axes[axis] += (2*velocity);
                //buzzer_set_period(1000);
                int redrawScreen = 1;
            }//*/

            if (shapeBoundary.topLeft.axes[0] < fence->topLeft.axes[0]) {
				newPos.axes[0] = screenWidth/2;
                newPos.axes[1] = screenHeight/2;
                ml->velocity.axes[0] = 2;
                ml->layer->posNext = newPos;
                lscore++;
                //drawString5x7(3,5, "Player 1", COLOR_GREEN, COLOR_WHITE);
                buzzer_set_period(2000);
                int redrawScreen = 1;
                //buzzer_set_period(0);
                break;
                
			}
			
			else if (shapeBoundary.botRight.axes[0] > fence->botRight.axes[0]) {
				newPos.axes[0] = screenWidth/2;
                newPos.axes[1] = screenHeight/2;
                ml->velocity.axes[0] = -2;
                ml->layer->posNext = newPos;
                rscore++;
                //drawString5x7(80,5, "Player 2", COLOR_GREEN, COLOR_WHITE);
                buzzer_set_period(2000);
                int redrawScreen = 1;
                //buzzer_set_period(0);
                break;
			}
            
			
			
			
		} /**< for ml */
		int redrawScreen = 1;
		ml->layer->posNext = newPos;
		if ( lscore > 9 || rscore > 9){
                layerDraw(&layer0);
                lscore = 0;
                rscore = 0;
        }
        score[0] = '0' + lscore;
        score[2] = '0' + rscore;
        p1 = p1->next;
        p2 = p2->next;
        
	}
	int redrawScreen = 1;
	drawString5x7(55,5, score , COLOR_WHITE, COLOR_BLACK);
    drawString5x7(3,5, "Player 1", COLOR_RED, COLOR_BLACK);
    drawString5x7(80,5, "Player 2", COLOR_PURPLE, COLOR_BLACK);
}


u_int bgColor = COLOR_BLACK;     /**< The background color */
int redrawScreen = 1;           /**< Boolean for whether screen needs to be redrawn */

Region fieldFence;		/**< fence around playing field  */


/** Initializes everything, enables interrupts and green LED, 
 *  and handles the rendering for the screen
 */
void movLeftDown(Layer *curLayers){
    Vec2 nextPos;
    Vec2 vel = {0,4};
    vec2Add(&nextPos, &curLayers->posNext, &vel);
    curLayers->posNext = nextPos;
}

void movLeftUp(Layer *curLayers){
    Vec2 nextPos;
    Vec2 vel = {0,-4};
    vec2Add(&nextPos, &curLayers->posNext, &vel);
    curLayers->posNext = nextPos;
}

void movRightDown(Layer *curLayers){
    Vec2 nextPos;
    Vec2 vel = {0,4};
    vec2Add(&nextPos, &curLayers->posNext, &vel);
    curLayers->posNext = nextPos;
}

void movRightUp(Layer *curLayers){
    Vec2 nextPos;
    Vec2 vel = {0,-4};
    vec2Add(&nextPos, &curLayers->posNext, &vel);
    curLayers->posNext = nextPos;
}

void main()
{

	P1DIR |= GREEN_LED;		/**< Green led on when CPU on */		
	P1OUT |= GREEN_LED;

	configureClocks();
    buzzer_init();
	lcd_init();
	shapeInit();
	p2sw_init(1);

	shapeInit();

	layerInit(&layer0);
	layerDraw(&layer0);


	layerGetBounds(&fieldLayer, &fieldFence);
	drawString5x7(55,5, score, COLOR_BLACK, COLOR_WHITE); //update to use variables

	enableWDTInterrupts();      /**< enable periodic interrupt */
	or_sr(0x8);	              /**< GIE (enable interrupts) */


	for(;;) { 
		while (!redrawScreen) { /**< Pause CPU if screen doesn't need updating */
			P1OUT &= ~GREEN_LED;    /**< Green led off witHo CPU */
			or_sr(0x10);	      /**< CPU OFF */
		}
		
		P1OUT |= GREEN_LED;       /**< Green led on when CPU on */
		redrawScreen = 0;
		movLayerDraw(&ml0, &layer0);
        movLayerDraw(&ml1, &layer0);
        movLayerDraw(&ml2, &layer0);
	}
}



/** Watchdog timer interrupt handler. 15 interrupts/sec */
void wdt_c_handler()
{
	static short count = 0;
	P1OUT |= GREEN_LED;		      /**< Green LED on when cpu on */
	count ++;
    
    Vec2 newPos;
	//u_char axis;
	Region shapeBoundary;
    
	if (count == 15) {
        
    /*Vec2 newPos;
	u_char axis;
	Region shapeBoundary;
    
    
	for (; ml; ml = ml->next) {
        //buzzer_set_period(0);
		vec2Add(&newPos, &ml->layer->posNext, &ml->velocity);
		abShapeGetBounds(ml->layer->abShape, &newPos, &shapeBoundary);
		for (axis = 0; axis < 2; axis ++) {
            if ((shapeBoundary.topLeft.axes[axis] < fence->topLeft.axes[axis]) || (shapeBoundary.botRight.axes[axis] > fence->botRight.axes[axis])) {
				int velocity = ml->velocity.axes[axis] = -ml->velocity.axes[axis];
                //buzzer_set_period(500);
				newPos.axes[axis] += (2*velocity);*/
        
		mlAdvance(&ml0, &ml1, &ml2, &fieldFence);
        if (states() == 1){
            //vec2Add(&newPos, &ml2->layer->posNext, &ml2->velocity);
            //abShapeGetBounds(ml2->layer->abShape, &newPos, &shapeBoundary);
            //if(shapeBoundary.botRight.axes[1] < fieldFence->botRight.axes[1]/*&ml0->layer->posNext < screenHeight*/){
            movLeftDown(&layer2);//}
        }
        else if (states() == 2){
            movLeftUp(&layer2);
        }
        else if (states() == 3){
            movRightDown(&layer1);
        }
        else if (states() == 4){
            movRightUp(&layer1);
        }
        redrawScreen = 1;
		count = 0;
	} 
	
	P1OUT &= ~GREEN_LED;		    /**< Green LED off when cpu off */
}
