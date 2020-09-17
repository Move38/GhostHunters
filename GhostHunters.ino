//Version Notes:
// using enumerated blinkTypes in some cases for color
//stores spawn rates in a list
//has WINTOKENS
//double click to start
#define PALE makeColorHSB(200,60,60)
#define lightHue 230
#define geistHue 135
#define bossHue 75
#define ghoulHue 8
#define DEAD_TIME 4500
#define GHOST_WAIT_TIME 2000
#define BOSS_TIME 3000
#define PERIOD 2000
#define SURVIVAL_TIME 50000 //one minute
#define INITIAL_SPAWN_TIME 500
#define WINTOKEN_SPAWN_CHANCE 90

// 100-these gives you the chance of spawn
byte BOSS_SPAWN_CHANCE;   //95 seems good 
byte GHOST_GHOUL_SPAWN_CHANCE;  //80 seems good
byte POLTER_SPAWN_CHANCE;

// A B C D E F
enum blinkType {EMPTY,GHOST,GHOUL,DEAD,WIN,LIGHT,BEAM,GEISTGUN,BOSS,POLTER,WINTOKEN};
byte blinkType=EMPTY;
enum signalState {LEVELSELECT,PLAY,GO,RESOLVE};
byte signalState=LEVELSELECT;
byte levelDifficulty=1;
bool source=false;
byte sendData;

Timer ghostWaitTimer;//when this runs out a new ghost may or may not spawn
Timer deadTimer; //whent this runs out you lose
Timer gameTimer;
Timer bossTimer;
Timer winTokenTimer;

byte receivingFace; //to orient the beam of light
byte dimness;
byte randomHaunting; //to see if haunted
byte ghoulOrGhost; //decides ghoul or ghost
byte receivedLevelDifficulty;
byte badBoiType;
byte badBoiHue[4]={lightHue,ghoulHue,geistHue,bossHue};
byte faceBlinkType[6]={EMPTY,EMPTY,EMPTY,EMPTY,EMPTY,EMPTY};
byte spawnRates[18]={80,80,75,80,80,75,0,15,17,0,15,20,101,101,101,97,97,90};
byte winTokenSpawnChance=101;
byte winFace=0;


void setup() {
  // put your setup code here, to run once:
  randomize();
}

void loop() {
   switch (signalState) {
    case LEVELSELECT:
      levelSelectLoop();
      break;
    case PLAY:
      PLAYLoop();
      break;
    case GO:
      goLoop();
      break;
    case RESOLVE:
      resolveLoop();
      break;
  }

  if(signalState!= LEVELSELECT){
    switch(blinkType){
      case WIN:
        winDisplay();
        break;
      case EMPTY:
        setColor(PALE);
        break;
      case GHOUL:
      case POLTER:
      case BOSS:
      case GHOST:
        badBoiDisplay();
        break;
      case BEAM:
      case GEISTGUN:
      case LIGHT:  
        beamsDisplay();
        break;
      case DEAD:
         deadDisplay();
         break;
      case WINTOKEN:
          winTokenDisplay();
          break;
    }
  }else{
    levelSelectDisplay();
  }

  if(signalState==LEVELSELECT){
    sendData = (levelDifficulty<<2) + signalState;
    setValueSentOnAllFaces(sendData);
  }else{
    if(!source){
      if(blinkType==LIGHT || blinkType==BEAM || blinkType==GEISTGUN){
        FOREACH_FACE(f){
          sendData = (faceBlinkType[f]<<2) + signalState;
          setValueSentOnFace(sendData,f);
        }
      }else{
        sendData = (blinkType<<2) + signalState;
        setValueSentOnAllFaces(sendData);
      }
    }else{
      sendData = (blinkType<<2) + signalState;
      setValueSentOnAllFaces(sendData);
    }
  }


 
}

void levelSelectLoop(){

  //WEAPON MAKER    Maybe double click while attached to start game?
  if(buttonDoubleClicked()){
        signalState=GO;
        source=true;
        blinkType=LIGHT;
    }

  //CHANGE TO PLAY STAGE
  if(buttonMultiClicked()){
    byte clicks=buttonClickCount();
    if(clicks==3){
      blinkType=EMPTY;
     
      signalState=GO;
    }
  }

//LEVEL TOGGLE
  if(buttonSingleClicked()){
    levelDifficulty++;
    if(levelDifficulty>6){
      levelDifficulty=1;
    }
  }

// use % here to make it so increase always happens and it just goes around the bend

    FOREACH_FACE(f){
      if(!isValueReceivedOnFaceExpired(f)){
        if(getSignalState(getLastValueReceivedOnFace(f))==LEVELSELECT){
          if(getLevelDifficulty(getLastValueReceivedOnFace(f)) != 0 && getLevelDifficulty(getLastValueReceivedOnFace(f))<7){
            receivedLevelDifficulty = getLevelDifficulty(getLastValueReceivedOnFace(f));
            if(levelDifficulty==1 && receivedLevelDifficulty==6){
              levelDifficulty = 1;
            }else if(levelDifficulty!=6 && levelDifficulty < receivedLevelDifficulty){
              levelDifficulty = receivedLevelDifficulty;
            }else if(levelDifficulty==6 && receivedLevelDifficulty==1){
              levelDifficulty=1;
            }
          }
        }
      }
    }

    //LISTEN FOR OTHERS IN PLAY STAGE
    FOREACH_FACE(f){
      if(!isValueReceivedOnFaceExpired(f)){
        if(getSignalState(getLastValueReceivedOnFace(f))==GO){
          signalState=GO;
        }
      }
    }
}

void PLAYLoop() {

  //set myself to LEVELSELECT
  if (buttonMultiClicked()) {
    if(!source){
      byte clicks=buttonClickCount();
      if(clicks==3){
        blinkType=WIN;
        signalState = RESOLVE;
        levelDifficulty=1;
      }
    }
  }
  
//------------------------------------
//   FLASHLIGHT AND LASER HANDLING
//------------------------------------

    if(buttonDoubleClicked()){
        if(!source){
          source=true;
          blinkType=LIGHT;
        }
    }

    //WHAT WEAPONS ARE AVAILABLE
     
    if(source==true){
      if(buttonPressed()){
        blinkType++;
        if(levelDifficulty<2 || levelDifficulty==4){
            if(blinkType>6){
               blinkType=5;
            }
          }else{
            if(blinkType>7){
               blinkType=5;
            }
          }
      }
    }
  
//WIN CONDITION and WINTOKENS
  if(gameTimer.isExpired()){
    if(levelDifficulty!=6){
      if(blinkType!=DEAD && source==false){
        blinkType=WIN;
      }
    }else{
      //WINTOKEN spawn
      if(winTokenTimer.isExpired()){
        if(WINTOKEN_SPAWN_CHANCE<random(100)){
           blinkType=WINTOKEN;
        }
        winTokenTimer.set(GHOST_WAIT_TIME);
      }
    }
  }

  if(blinkType==WINTOKEN){
      if(!isValueReceivedOnFaceExpired(winFace)){
        if(getBlinkType(getLastValueReceivedOnFace(winFace))==LIGHT){
          blinkType==WIN;
        }
      }
  }


  

  if(blinkType!=DEAD){
    FOREACH_FACE(f){
      if(!isValueReceivedOnFaceExpired(f)){
        if(getBlinkType(getLastValueReceivedOnFace(f))==WIN){
          blinkType==WIN;
        }
      }
    }
  }
  
 //if im not dead,check to see if someone near me is dead. if so. same.
  
  if(blinkType!=DEAD && source==false){
    FOREACH_FACE(f){
      if(!isValueReceivedOnFaceExpired(f)){
        if(getBlinkType(getLastValueReceivedOnFace(f))==DEAD){
          gameTimer.set(0);
          blinkType=DEAD;
        }
      }
    }
  }

  //GHOST AND GHOUL SPAWNING AND ALSO SOME POLTERGEISTS!
  if(blinkType==EMPTY){
    if(noGhostNeighbors){
    if(ghostWaitTimer.isExpired()){
      //check for neighborGhosts. dont spawn a ghost if neighbor is a ghosts 
      randomHaunting=random(100);
      ghoulOrGhost=(random(100));
        if(randomHaunting>=GHOST_GHOUL_SPAWN_CHANCE){  //CHANGE TO ADJUST SPAWN RATE
          if(55<ghoulOrGhost){
            blinkType=GHOUL;
            badBoiType=1;
            deadTimer.set(DEAD_TIME);
          }else{
            blinkType=GHOST;
            badBoiType=0;
            deadTimer.set(DEAD_TIME);
          }
            deadTimer.set(DEAD_TIME);
        }else if(randomHaunting<POLTER_SPAWN_CHANCE){
          blinkType=POLTER;
          deadTimer.set(DEAD_TIME);
          badBoiType=2;
        }
      
      ghostWaitTimer.set(GHOST_WAIT_TIME);
      //ghostWaitTimer.set(random(500)+RANDOM_GHOST_TIME);
    }
  
    //BOSS SPAWNIN
    if(bossTimer.isExpired()){
      randomHaunting=random(100);
        if(randomHaunting>=BOSS_SPAWN_CHANCE){  //CHANGE TO ADJUST SPAWN RATE
          blinkType=BOSS;
          badBoiType=3;
          deadTimer.set(DEAD_TIME);
        }
      bossTimer.set(BOSS_TIME);
    }
  }


}
  
//----------------
//   KILLING MOBS 
//----------------
  if(blinkType==BOSS){
    if(isReceivingLaser() && isReceivingLight()){
      blinkType=EMPTY;
    }
  }

  if(blinkType==GHOST){
    if(isReceivingLight()){
      blinkType=EMPTY;
    }
  }

  if(blinkType==POLTER){
    FOREACH_FACE(f){
      if(!isValueReceivedOnFaceExpired(f)){
        if(getBlinkType(getLastValueReceivedOnFace(f))==GEISTGUN){
          blinkType=EMPTY;
        }
     }
    }
  }

  if(blinkType==GHOUL){
    if(isReceivingLaser()){
      blinkType=EMPTY;
    }
  }

//-----------------------------------------
// Super Clunky Signal Sending Chunk (SCSC)
//-----------------------------------------

  //light sending
  if(blinkType==LIGHT && source==false){
    if(!isValueReceivedOnFaceExpired(receivingFace)){
      if(getBlinkType(getLastValueReceivedOnFace(receivingFace))==LIGHT){
        blinkType=LIGHT;
        faceBlinkType[(receivingFace+3)%6]=LIGHT;
      }else{
        blinkType=EMPTY;
        faceBlinkType[(receivingFace+3)%6]=EMPTY;
      }
    }else{
      blinkType=EMPTY;
      faceBlinkType[(receivingFace+3)%6]=EMPTY;
    }
  }
  
  
  //laser sending
  if(blinkType==BEAM && source==false){
    if(!isValueReceivedOnFaceExpired(receivingFace)){
      if(getBlinkType(getLastValueReceivedOnFace(receivingFace))==BEAM){
        blinkType=BEAM;
        faceBlinkType[(receivingFace+3)%6]=BEAM;
      }else{
        blinkType=EMPTY;
        faceBlinkType[(receivingFace+3)%6]=EMPTY;
      }
    }else{
      blinkType=EMPTY;
      faceBlinkType[(receivingFace+3)%6]=EMPTY;
    }
  }

  //GEIST GUN SENDING
  if(blinkType==GEISTGUN && source==false){
    if(!isValueReceivedOnFaceExpired(receivingFace)){
      if(getBlinkType(getLastValueReceivedOnFace(receivingFace))==GEISTGUN){
        blinkType=GEISTGUN;
        faceBlinkType[(receivingFace+3)%6]=GEISTGUN;
      }else{
        blinkType=EMPTY;
        faceBlinkType[(receivingFace+3)%6]=EMPTY;
      }
    }else{
      blinkType=EMPTY;
      faceBlinkType[(receivingFace+3)%6]=EMPTY;
    }
  }

  //From empty to beam
  if(blinkType==EMPTY){
    FOREACH_FACE(f){
        if(!isValueReceivedOnFaceExpired(f)){
          byte beamType = getBlinkType(getLastValueReceivedOnFace(f));
          if(beamType == LIGHT || beamType == BEAM || beamType == GEISTGUN){
            receivingFace=f;
            blinkType=beamType;
            faceBlinkType[(receivingFace+3)%6]=beamType;
          }
       }
     }
  }

  //IF I DONT KILL THE GHOSTS OR GHOULS FAST ENOUGH I DIE
  if(blinkType==GHOST || blinkType==GHOUL || blinkType==BOSS || blinkType==POLTER){
    if(deadTimer.isExpired()){
      blinkType=DEAD;
    }
  }

  //listen for neighbors in resolve
  FOREACH_FACE(f) {
    if (!isValueReceivedOnFaceExpired(f)) {//a neighbor!
      if (getSignalState(getLastValueReceivedOnFace(f)) == RESOLVE) {//a neighbor saying GO!
        signalState = RESOLVE;

      }
    }
  }
}

void goLoop() {
  signalState = PLAY;

  if(!source){
    blinkType=EMPTY;
  }
  
  bossTimer.set(BOSS_TIME);
  ghostWaitTimer.set((random(INITIAL_SPAWN_TIME)*3));
  GHOST_GHOUL_SPAWN_CHANCE=spawnRates[levelDifficulty-1];
  POLTER_SPAWN_CHANCE=spawnRates[6+levelDifficulty-1];
  BOSS_SPAWN_CHANCE=spawnRates[12+levelDifficulty-1];
  
  gameTimer.set(SURVIVAL_TIME);

  //look for neighbors who have not heard the news
  FOREACH_FACE(f) {
    if (!isValueReceivedOnFaceExpired(f)) {//a neighbor!
      if (getSignalState(getLastValueReceivedOnFace(f)) == LEVELSELECT) {
        signalState = GO;
      }
    }
  }
}

//------------------------------------------
//    RESOLVE transitions to LEVELSELECT
//------------------------------------------

void resolveLoop() {
  signalState = LEVELSELECT;//I default to this at the start of the loop. Only if I see a problem does this not happen

  blinkType=EMPTY;
  source=false;
  //levelDifficulty=1;

  //look for neighbors who have not moved to RESOLVE
  FOREACH_FACE(f) {
    if (!isValueReceivedOnFaceExpired(f)) {//a neighbor!
      if (getSignalState(getLastValueReceivedOnFace(f)) == PLAY) {//This neighbor isn't in RESOLVE. Stay in RESOLVE
        signalState = RESOLVE;
      }
    }
  }
}

//For lasers
bool isReceivingLaser(){
  byte lasers=0;
  FOREACH_FACE(f){
    if (!isValueReceivedOnFaceExpired(f)) {//a neighbor!
      if (getBlinkType(getLastValueReceivedOnFace(f)) == BEAM){
        lasers++;
      }
    }
  }
  if(lasers==0){
    return false;
  }else{
    return true;
  }
}

//For lights
bool isReceivingLight(){
  byte lights=0;
  FOREACH_FACE(f){
    if (!isValueReceivedOnFaceExpired(f)) {//a neighbor!
      if (getBlinkType(getLastValueReceivedOnFace(f)) == LIGHT){
        lights++;
      }
    }
  }
  if(lights==0){
    return false;
  }else{
    return true;
  }
}

void breath(){
  byte breathProgress = map(millis()%PERIOD,0,PERIOD,0,255);
  dimness = (sin8_C(breathProgress)/2)+127;
}


void beamsDisplay(){
  byte sat=240;
  if(blinkType==LIGHT){
    sat=0;
  }
  if(source){
    setColor(makeColorHSB(badBoiHue[blinkType-5],sat,255));
  }else{
    if(blinkType==LIGHT){
      setColor(WHITE);
    }else if(blinkType==BEAM){
      setColor(OFF);
      setColorOnFace(makeColorHSB(badBoiHue[blinkType-5],sat,255),receivingFace);
      setColorOnFace(makeColorHSB(badBoiHue[blinkType-5],240,255),(receivingFace+3)%6);
    }else{
      setColor(makeColorHSB(badBoiHue[blinkType-5],random(50)+190,random(70)+70));
      setColorOnFace(makeColorHSB(badBoiHue[blinkType-5],sat,255),receivingFace);
      setColorOnFace(makeColorHSB(badBoiHue[blinkType-5],sat,255),(receivingFace+3)%6);
    }
  }
}


void deadDisplay(){
  breath();
  setColor(makeColorHSB(15,random(70)+170,dimness));
}

void winDisplay(){
     breath();
     setColor(makeColorHSB(45,random(70)+170,dimness));
}


void badBoiDisplay(){
  byte sat=255;
  if(blinkType==GHOST){
    sat=0;
  }
  breath();
  byte badFaces = map(deadTimer.getRemaining(),0,DEAD_TIME,0,5);
  
  setColor(makeColorHSB(badBoiHue[badBoiType],sat,dimness));
  
  FOREACH_FACE(f){
    if(f<=badFaces-1){
      setColorOnFace(makeColorHSB(badBoiHue[badBoiType],sat,dimness-120),f);
    }
  }
}

//DISPLAY CURRENT LEVEL
void levelSelectDisplay(){
  setColor(PALE);
  FOREACH_FACE(f){
    if(f<levelDifficulty){
      setColorOnFace(WHITE,f);
    }
  }
}

void winTokenDisplay(){
  if(ghostWaitTimer.isExpired()){
    winFace++;
    if(winFace>5){
      winFace=0;
    }
    ghostWaitTimer.set(INITIAL_SPAWN_TIME);
  }
  setColor(OFF);
  setColorOnFace(YELLOW,winFace);
}

bool noGhostNeighbors(){
  byte neighbors=0;
  FOREACH_FACE(f) {
    if (!isValueReceivedOnFaceExpired(f)) {//a neighbor!
      if (getBlinkType(getLastValueReceivedOnFace(f)) != EMPTY) {
          neighbors++;
      }
    }
  }
  if(neighbors>0){
    return true;
  }else{
    return false;
  }
}


byte getBlinkType(byte data){
  return (data>>2);
}

byte getSignalState(byte data){
  return (data & 3);
}

byte getLevelDifficulty(byte data){
  return (data>>2);
}
