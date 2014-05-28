/*


* Parser.cpp
 *
 *  Created on: 30.04.2014
 *      Author: LeonBornemann
 */


#include <frontend/parse/Lexer.h>
#include <frontend/Parser.h>


std::list<char> listFromArray(char chars[], int size){
	std::list<char> myList;
	for(int i=0;i<size;i++){
		myList.push_back(chars[i]);
	}
	return myList;
}

Parser::Parser(std::shared_ptr<RailFunction> railFunction) {
	this->board = railFunction;
	this->xlen = railFunction->getWidth();
	this->ylen = railFunction->getHeight();
	this->graphName = railFunction->getName();
	parsingNotFinished = true;
	//errorMessage - empty String: Everything is ok
	errorMessage = "";
	abstractSyntaxGraph = NULL;
	//Order in offsetvalues: [0] - Left, [1] - straight, [2] - right
	initializeOffsetMaps();
	//allowedChars
	initializeValidRailMap();
	//if the train moves straight but reads a specific character the direction needs to be changesd
	initializeDirChangeMaps();

}

void Parser::setXY(int newX, int newY){
	posX = newX;
	posY = newY;
}

shared_ptr<Adjacency_list> Parser::parseGraph() {
	addNextNodeAsTruePathOfPreviousNode = true;
	int startPosY=-1;
	//find $, initiate pos
	for(int i =0;i<ylen;++i) {
		if(board->get(0, i) == '$') {
			startPosY = i;
			break;
		}
	}
	if(startPosY==-1){
		errorMessage ="$ not found in function "+graphName;
		return NULL;
	}
	return parseGraph(0,startPosY,SE);
}

shared_ptr<Adjacency_list> Parser::parseGraph(int startPosX, int startPosY, Direction startDir){
	cout << "Begin parsing..." << endl;
	setXY(startPosX,startPosY);
	dir = startDir;
	parsingNotFinished = true;
	while(parsingNotFinished) {
		cout << "\t@(" << posX << ", " << posY << ", " << Encoding::unicodeToUtf8(board->get(posX, posY)) << ")" << endl;
		move();
		if(errorMessage != ""){
			cout << "\t" << errorMessage <<endl;
			cout << "\tparsing aborted" << endl;
			break;
		}
	}
	cout << "Finished parsing" << endl;
	return abstractSyntaxGraph;
}

void Parser::move(){
	//straight
	int straightX = posX + xOffsetMap[dir].offsets[STRAIGHT];
	int straightY = posY + yOffsetMap[dir].offsets[STRAIGHT];
	bool straightIsInBoardBounds = straightX >=0 && straightX < xlen && straightY >=0 && straightY <ylen;
	//left
	int leftX = posX + xOffsetMap[dir].offsets[LEFT];
	int leftY = posY + yOffsetMap[dir].offsets[LEFT];
	bool leftIsInBoardBounds =  leftX >=0 && leftX < xlen && leftY >=0 && leftY <ylen;
	//right
	int rightX = posX + xOffsetMap[dir].offsets[RIGHT];
	int rightY = posY + yOffsetMap[dir].offsets[RIGHT];
	bool rightIsInBoardBounds =  rightX >=0 && rightX < xlen && rightY >=0 && rightY <ylen;
	//bool vars that will be checked in the end
	bool leftIsValidRail = false;
	bool rightIsValidRail = false;
	bool straightIsValidRail = false;
	if(straightIsInBoardBounds) {
		char charAtStraight = board->get(straightX, straightY);
		list<char> allowedRails = validRailMap[dir].straight;
		straightIsValidRail = std::find(allowedRails.begin(),allowedRails.end(),charAtStraight)!=allowedRails.end();
		if(straightIsValidRail){
			//if allowedRails contains charAtStraight just move forwards
			setXY(straightX,straightY);
			if(charAtStraight == leftDirChangeMap[dir]){
				turnLeft45Deg();
			}
			if(charAtStraight == rightDirChangeMap[dir]){
				turnRight45Deg();
			}
			return;
		}
		//check for other symbols that are allowed
		bool didGoStraight = checkForValidCommandsInStraightDir(straightX,straightY);
		if(didGoStraight){
			return;
		}
	}
	if(leftIsInBoardBounds){

		char charAtLeft = board->get(leftX, leftY);
		list<char> allowedRailsLeft = validRailMap[dir].left;
		leftIsValidRail = std::find(allowedRailsLeft.begin(),allowedRailsLeft.end(),charAtLeft)!=allowedRailsLeft.end();
	}
	if(rightIsInBoardBounds){
		char charAtRight = board->get(rightX, rightY);
		list<char> allowedRailsRight = validRailMap[dir].right;
		rightIsValidRail = std::find(allowedRailsRight.begin(),allowedRailsRight.end(),charAtRight)!=allowedRailsRight.end();
	}
	//error handling begin
	if(leftIsValidRail && rightIsValidRail){
		std::stringstream sstm;
		sstm << "abiguous move at line" << posX << ", character:" << posY;
		errorMessage = sstm.str();
		return;
	}
	if(!leftIsValidRail && !rightIsValidRail){
		std::stringstream sstm;
		sstm << "no valid move possible at line" << posX << ", character:" << posY;
		errorMessage = sstm.str();
		return;
	}
	//error handling end
	if(leftIsValidRail){
		setXY(leftX,leftY);
		turnLeft45Deg();
		return;
	}
	if(rightIsValidRail){
		setXY(rightX,rightY);
		turnRight45Deg();
		return;
	}
	errorMessage = "end of move-function reached - this should never happen and is an internal error";
	return;
}

bool Parser::checkForValidCommandsInStraightDir(int straightX, int straightY){
	char charAtStraight = board->get(straightX, straightY);
	bool didGoStraight = true;
	string toPush;
	switch(charAtStraight){
	case 'o':
		setXY(straightX,straightY);
		addToAbstractSyntaxGraph("o",Command::Type::OUTPUT);
		break;
	case '[':
		setXY(straightX,straightY);
		//TODO: ueberpruefen ob notwendig: list<char> invalidCharList = listFromArray({'[','{','(',},);
		toPush = readCharsUntil(']');
		addToAbstractSyntaxGraph(toPush,Command::Type::PUSH_CONST);
		//TODO: create pushNode in graph
		break;
	case ']':
		setXY(straightX,straightY);
		toPush = readCharsUntil('[');
		addToAbstractSyntaxGraph(toPush,Command::Type::PUSH_CONST);
		break;
	case '@':
		setXY(straightX,straightY);
		reverseDirection();
		break;
	case '#':
		setXY(straightX,straightY);
		addToAbstractSyntaxGraph("#",Command::Type::FINISH);
		parsingNotFinished = false;
		break;
	case '<':
		didGoStraight = parseJunctions(E,straightX,straightY,SE,NE,"<",Command::Type::EASTJUNC);
		break;
	case '>':
		didGoStraight = parseJunctions(W,straightX,straightY,NW,SW,">",Command::Type::WESTJUNC);
		break;
	case '^':
		didGoStraight = parseJunctions(S,straightX,straightY,SW,SE,"^",Command::Type::SOUTHJUNC);
		break;
	case 'v':
		didGoStraight = parseJunctions(N,straightX,straightY,NE,NW,"v",Command::Type::NORTHJUNC);
		break;
	default:
		didGoStraight = false;
		break;
	}
	if(errorMessage!=""){
		//TODO:Error stuff?
	}
	return didGoStraight;
}

/*
 * requiredDir: The direction the train has to have at the moment for this being a valid junction (for dir must be E for < junction)
 * juncX/juncY are the coordinates of the junction
 * truePathDir/falsePathDir are the starting directions of the true/false paths
 * command name: the junction symbol as a string
 * juncType: ast.h Command::type enum value of the junction
*/
bool Parser::parseJunctions(Direction requiredDir,int juncX,int juncY,Direction truePathDir,Direction falsePathDir,string commandName,Command::Type juncType){
	if(dir==requiredDir){
		addToAbstractSyntaxGraph(commandName,juncType);
		std::shared_ptr<Node> ifNode = currentNode;
		parseGraph(juncX,juncY,truePathDir);
		parsingNotFinished = true;
		currentNode = ifNode;
		addNextNodeAsTruePathOfPreviousNode = false;
		parseGraph(juncX,juncY,falsePathDir);
		parsingNotFinished = false;
		return true;
	} else{
		return false;
	}
}

void Parser::addToAbstractSyntaxGraph(string commandName,Command::Type type){
	//debug
	cout << "\tNode creation: " << commandName << endl;
	std::shared_ptr<Node> node(new Node());
	node->command = {type,commandName};
	//TODO:an Graph Schnittstelle anpassen
	if(abstractSyntaxGraph == NULL){
		//this is the first node that we meet create a new one
		node->id = 1;
		lastUsedId = 1;
		abstractSyntaxGraph.reset(new Adjacency_list(graphName,node));
	} else{
		node->id = ++lastUsedId;
		abstractSyntaxGraph->addNode(node);
		abstractSyntaxGraph->addEdge(currentNode,node,addNextNodeAsTruePathOfPreviousNode);
		if(!addNextNodeAsTruePathOfPreviousNode){
			//restore default behavior: always add new nodes to the 'true' path of their predecessor
			//unless it was set before the call of this method (in case an 'IF' was read and we parse the first node of the 'false' branch
			addNextNodeAsTruePathOfPreviousNode = true;
		}
	}
	currentNode = node;
}


//setzt position auf until falls er existiert, und gibt den gelesenen string inklusive anfangs und endzeichen zurueck
//falls nicht wir ein leerer string zurueckgegeben und die fehlermeldung gesetzt
string Parser::readCharsUntil(char until) {
	string result = "";
	result += board->get(posX, posY);
	while(true){
		int nextX = posX + xOffsetMap[dir].offsets[STRAIGHT];
		int nextY = posY + yOffsetMap[dir].offsets[STRAIGHT];
		if(posX >= xlen || nextY >= ylen){
			//TODO:Dir auch ausgeben
			std::stringstream sstm;
			sstm << "Parsing ran out of valid space for function in line" << posX << ", character:" << posY;
			errorMessage = sstm.str();
			return "";
		}
		posX = nextX;
		posY = nextY;
		result += board->get(posX, posY);
		if(board->get(posX, posY) == until) {
			break;
		}
	}
	return result;
}

void Parser::turnLeft45Deg(){
	switch(dir){
	case E: dir = NE; break;
	case SE: dir = E; break;
	case S: dir = SE; break;
	case SW: dir = S; break;
	case W: dir = SW; break;
	case NW: dir = W; break;
	case N: dir = NW; break;
	case NE: dir = N; break;
	}
}

void Parser::turnRight45Deg(){
	switch(dir){
		case E: dir = SE; break;
		case SE: dir = S; break;
		case S: dir = SW; break;
		case SW: dir = W; break;
		case W: dir = NW; break;
		case NW: dir = N; break;
		case N: dir = NE; break;
		case NE: dir = E; break;
		}
}

void Parser::reverseDirection(){
	switch(dir){
	case E: dir = W; break;
	case SE: dir = NW; break;
	case S: dir = N; break;
	case SW: dir = NE; break;
	case W: dir = E; break;
	case NW: dir = SE; break;
	case N: dir = S; break;
	case NE: dir = SW; break;
	}
}

//int calcXOffsetStraight
void Parser::initializeOffsetMaps(){
	/*
	 * The xOffsetMap/yOffsetMap are responsible for providing the offsets(based on the current position and Direction) that tell you where to look for the character
	 * For Example if you are going east and you want to look left, x(rowNumber) needs to be lowered by 1, if you are going straight x stays the same(0), if you are going right x needs to be increased by 1 (+1)
	 * Thus the offset-values are saved as a triple of ints, first one being for left, second for straight, third right
	 */
	//x offsets
	xOffsetMap[E] = offsetvalues{ {-1,0,+1} };  //Direction E
	xOffsetMap[SE] = offsetvalues{ {0,+1,+1} }; //Direction: SE
	xOffsetMap[S] = offsetvalues{ {+1,+1,+1} }; //Direction: S
	xOffsetMap[SW] = offsetvalues{ {+1,+1,0} };//Direction: SW
	xOffsetMap[W] = offsetvalues{ {+1,0,-1} };  //Direction: W
	xOffsetMap[NW] = offsetvalues{ {0,-1,-1} }; //Direction: NW
	xOffsetMap[N] = offsetvalues{ {-1,-1,-1} }; //Direction: N
	xOffsetMap[NE] = offsetvalues{ {-1,-1,0} };  //Direction: NE
	//y offsets:
	yOffsetMap[E] = offsetvalues{ {+1,+1,+1} };  //DIrection E
	yOffsetMap[SE] = offsetvalues{ {+1,+1,0} }; //Direction: SE
	yOffsetMap[S] = offsetvalues{ {+1,0,-1} }; //Direction: S
	yOffsetMap[SW] = offsetvalues{ {0,-1,-1} };//Direction: SW
	yOffsetMap[W] = offsetvalues{ {-1,-1,-1} };  //Direction: W
	yOffsetMap[NW] = offsetvalues{ {-1,-1,0} }; //Direction: NW
	yOffsetMap[N] = offsetvalues{ {-1,0,+1} }; //Direction: N
	yOffsetMap[NE] = offsetvalues{ {0,+1,+1} };  //Direction: NE
}

void Parser::initializeValidRailMap(){
	/*
	 * The validRailMap is responsible for identifying valid rails when going into a specific direction
	 * Naturally the valid rails differ from looking left, straight or right.
	 * For example if the train is moving east, when looking straight you can either read '-','/','\','+' or '*' as a valid rail
	 * (Some of these may result in a change of direction but this is handled by leftDirChangeMap and rightDirChangeMap)
	 * Taking a left or right turn will always result in a change of direction(so there are no maps for this case)
	 */
	//East
	char Eleft[] = {'/','*','x'};
	char EStraight[] = {'-','/','\\','+','*'};
	char ERight[] = {'\\','*','x'};
	validRailMap[E] = allowedChars{listFromArray(Eleft,2),listFromArray(EStraight,5),listFromArray(ERight,2)};
	//Southeast
	char SEleft[] = {'-','*','+'};
	char SEStraight[] = {'-','\\','|','*','x'};
	char SERight[] = {'|','*','+'};
	validRailMap[SE] = allowedChars{listFromArray(SEleft,3),listFromArray(SEStraight,4),listFromArray(SERight,3)};
	//South
	char Sleft[] = {'\\','*','x'};
	char SStraight[] = {'|','\\','/','*','+'};
	char SRight[] = {'/','*','x'};
	validRailMap[S] = allowedChars{listFromArray(Sleft,2),listFromArray(SStraight,5),listFromArray(SRight,2)};
	//Southwest
	char SWleft[] = {'|','*','+'};
	char SWStraight[] = {'-','/','|','*','x'};
	char SWRight[] = {'-','*','+'};
	validRailMap[SW] = allowedChars{listFromArray(SWleft,3),listFromArray(SWStraight,4),listFromArray(SWRight,3)};
	//West
	char Wleft[] = {'/','*','x'};
	char WStraight[] = {'-','/','\\','+','*'};
	char WRight[] = {'\\','*','x'};
	validRailMap[W] = allowedChars{listFromArray(Wleft,2),listFromArray(WStraight,5),listFromArray(WRight,2)};
	//Northwest
	char NWleft[] = {'-','*','+'};
	char NWStraight[] = {'-','\\','|','*','x'};
	char NWRight[] = {'|','*','+'};
	validRailMap[NW] = allowedChars{listFromArray(NWleft,3),listFromArray(NWStraight,4),listFromArray(NWRight,3)};
	//North
	char Nleft[] = {'\\','*','x'};
	char NStraight[] = {'|','\\','/','*','+'};
	char NRight[] = {'/','*','x'};
	validRailMap[N] = allowedChars{listFromArray(Nleft,2),listFromArray(NStraight,5),listFromArray(NRight,2)};
	//Northeast
	char NEleft[] = {'|','*','+'};
	char NEStraight[] = {'-','/','|','*','x'};
	char NERight[] = {'-','*','+'};
	validRailMap[NE] = allowedChars{listFromArray(NEleft,3),listFromArray(NEStraight,4),listFromArray(NERight,3)};
}

void Parser::initializeDirChangeMaps(){
	/*
	 * The leftDirChangeMap/rightDirChangeMap are needed for the following case:
	 * The train did go straight and now it needs to be decided, if the direction needs to be changed
	 * For Example if your Direction is East and you go straight by reading '/' you need to change the direction (left turn)
	 */
	//when to turn left 45 deg
	leftDirChangeMap[E] = '/';
	leftDirChangeMap[SE] = '-';
	leftDirChangeMap[S] = '\\';
	leftDirChangeMap[SW] = '|';
	leftDirChangeMap[W] = '/';
	leftDirChangeMap[NW] = '-';
	leftDirChangeMap[N] = '\\';
	leftDirChangeMap[NE] = '|';
	//when to turn right 45 deg
	rightDirChangeMap[E] = '\\';
	rightDirChangeMap[SE] = '|';
	rightDirChangeMap[S] = '/';
	rightDirChangeMap[SW] = '-';
	rightDirChangeMap[W] = '\\';
	rightDirChangeMap[NW] = '|';
	rightDirChangeMap[N] = '/';
	rightDirChangeMap[NE] = '-';
}

