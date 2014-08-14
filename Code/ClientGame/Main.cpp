
#include "stdafx.h"
#include <pthread.h>
#include <sstream>

#include "HudTextActor.h"

using namespace std;

class CameraMessageListener : public MessageListener {
public:
    
    virtual void ReceiveMessage(Message* m);
    
    CameraMessageListener() {
        added = false;
        t = new TextActor("Console", "Here I am \nRock you like a hurricane", TXT_Center);
    }
    
    TextActor *t;
    bool added;
    
};

class Soldier {
public:
    
    Soldier(float x=0.0f, float y=0.0f) {
        figure = new Actor();
        figure->SetSprite("Resources/Images/soldierA_000.png");
        figure->LoadSpriteFrames("Resources/Images/soldierA_000.png");
        figure->PlaySpriteAnimation(0.75f, SAT_Loop, 0, 1);
        figure->SetPosition(x, y);
        theWorld.Add(figure,2);
        
        count = new Actor();
        count->SetPosition(x, y);
        count->SetSprite("Resources/Images/10.png");
        theWorld.Add(count,3);
        
    }
    
    
    void MoveTo(float x, float y) {
    }
    
    Actor* figure;
    Actor* count;
};


class Cursor : public MessageListener{
public:
    Cursor() {
        cursor = new Actor();
        cursor->SetSprite("Resources/Images/cursor.png");
        cursor->SetSize(1.25f);
        cursor->SetPosition(3,3);
        cursor->SetLayer(5);
        theWorld.Add(cursor);
        
        moving = false;
        
        //pthread_mutex_init(&message_lock, NULL);
        
        theSwitchboard.SubscribeTo(this,  "CursorMoveComplete");
        
    }
    
    virtual void ReceiveMessage(Message* m) {
        
        if (m->GetMessageName().compare("CursorMoveComplete")==0) {
            moving = false;
            return;
        }
        
        if (moving == true) {
            return;
        }
        
        Vector2 new_position(cursor->GetPosition());
        if (m->GetMessageName().compare("Up")==0) {
            new_position.Y+=1.0f;
        }
        else if (m->GetMessageName().compare("Down")==0) {
            new_position.Y-=1.0f;
        }
        else if (m->GetMessageName().compare("Right")==0) {
            new_position.X+=1.0f;
        }
        else if (m->GetMessageName().compare("Left")==0) {
            new_position.X-=1.0f;
        }
        
        moving = true;
        cursor->MoveTo(new_position, 0.2, false,  "CursorMoveComplete");
        
        //pthread_mutex_unlock(&message_lock);
    }
    
private:
    
    Actor* cursor;
    bool moving;
    //pthread_mutex_t message_lock;
    
};

class Board {
    Vec2i board_size;
    int** tiles;
public:
    Board(Vec2i size) {
        board_size = size;
        tiles = new int*[board_size.X];
        for (int i=0; i<board_size.X; i++) {
            tiles[i] = new int[board_size.Y];
            for (int j=0;j<board_size.Y;j++) {
                tiles[i][j] = rand()%3;
            }
        }
        
        char* tile_names[] = {"Resources/images/roadA.png", "Resources/images/treesA.png", "Resources/images/grassA.png"};
        //std::vector<String> tile_names(
        for (int i=0; i<board_size.X; i++) {
            for (int j=0; j<board_size.Y; j++) {
                Actor* tile = new Actor();
                tile->SetPosition(i, j);
                char* tile_name = tile_names[tiles[i][j]];
                tile->SetSprite(tile_name);
                theWorld.Add(tile,1);
            }
        }
    }
    
    float cost(Vec2i from, Vec2i to) {
        if (tiles[to.X][to.Y]==0)  {
            return 1.0;
        } else if (tiles[to.X][to.Y]==1)  {
            return 2.0;
        } else if (tiles[to.X][to.Y]==2)  {
            return 2.0;
        }
        return 10.0;
    }
    
    Vec2i GetBoardSize() {
        return board_size;
    }
};

class Dialog {
public:
    virtual ~Dialog() {
    }
    
    virtual bool HandleMessage(Message* m) {
        sysLog.Log("message recieved");
        
        return true;
    }
};

class WindowManager : public MessageListener {
    
public:
    WindowManager() {
        theSwitchboard.SubscribeTo(this,  "Up");
        theSwitchboard.SubscribeTo(this,  "Down");
        theSwitchboard.SubscribeTo(this,  "Left");
        theSwitchboard.SubscribeTo(this,  "Right");
        theSwitchboard.SubscribeTo(this,  "AButton");
        theSwitchboard.SubscribeTo(this,  "BButton");
    }
    
    virtual void ReceiveMessage(Message* m) {
        if (dialogStack.size() > 0) {
            Dialog* dialog = dialogStack.back();
            if (dialog != NULL) {
                if (dialog->HandleMessage(m)) {
                    return;
                }
            }
        }
    }
    
    void PushDialog(Dialog* d) {
        dialogStack.push_back(d);
    }
    
    void PopDialog() {
        Dialog* d = dialogStack.back();
        dialogStack.pop_back();
        delete d;
    }
    
    static WindowManager* GetWindowManager() {
        return windowManager;
    }
    
private:
    static WindowManager* windowManager;
    vector<Dialog*> dialogStack;
};


WindowManager* WindowManager::windowManager = new WindowManager();




class BlueDialog : public Dialog {
public:
    BlueDialog(Vec2i topLeft, int layer) {
        this->layer = layer;
        this->topLeft = topLeft;
        size = Vec2i(200,300);
        position = Vec2i(topLeft.X+size.X/2,topLeft.Y+size.Y/2);
        
        this-> background = new HUDActor();
        background->SetPosition(position.X,position.Y);
        background->SetSize(size);
        background->SetLayer(this->layer-1);
        background->SetColor(0.0, 0.0, 0.5);
        theWorld.Add(background);
        
        leftBorder = new HUDActor();
        leftBorder->SetPosition(position.X-(size.X/2),position.Y);
        leftBorder->SetSize(Vector2(6,size.Y));
        leftBorder->SetSprite("Resources/Images/border.png", 0, GL_REPEAT);
        leftBorder->SetLayer(this->layer);
        leftBorder->SetUVs(Vector2(0,0), Vector2(1,size.Y/16.0));
        theWorld.Add(leftBorder);
        
        rightBorder = new HUDActor();
        rightBorder->SetPosition(position.X+(size.X/2),position.Y);
        rightBorder->SetSize(Vector2(6,size.Y));
        rightBorder->SetSprite("Resources/Images/border.png", 0, GL_REPEAT);
        rightBorder->SetLayer(this->layer);
        rightBorder->SetUVs(Vector2(0,0), Vector2(1,size.Y/16.0));
        theWorld.Add(rightBorder);
        
        topBorder = new HUDActor();
        topBorder->SetPosition(position.X,position.Y-size.Y/2);
        topBorder->SetSize(Vector2(6,size.X));
        topBorder->SetSprite("Resources/Images/border.png", 0, GL_REPEAT);
        topBorder->SetLayer(this->layer);
        topBorder->SetUVs(Vector2(0,0), Vector2(1,size.X/16.0));
        topBorder->SetRotation(90.0);
        theWorld.Add(topBorder);
        
        bottomBorder = new HUDActor();
        bottomBorder->SetPosition(position.X,position.Y+size.Y/2);
        bottomBorder->SetSize(Vector2(6,size.X));
        bottomBorder->SetSprite("Resources/Images/border.png", 0, GL_REPEAT);
        bottomBorder->SetLayer(this->layer);
        bottomBorder->SetUVs(Vector2(0,0), Vector2(1,size.X/16.0));
        bottomBorder->SetRotation(90.0);
        theWorld.Add(bottomBorder);
        
        for (int i=1; i<4; i++) {
            HudTextActor* ht = new HudTextActor("draw me", "Inconsolata", topLeft.X + 30, topLeft.Y + i*30);
            ht->SetLayer(this->layer);
            ht->SetColor(1.0, 1.0, 1.0);
            theWorld.Add(ht);
            textCommands.push_back(ht);
        }
    }
    
    ~BlueDialog() {
        theWorld.Remove(background);
        theWorld.Remove(leftBorder);
        theWorld.Remove(rightBorder);
        theWorld.Remove(topBorder);
        theWorld.Remove(bottomBorder);
        delete(background);
        delete(leftBorder);
        delete(rightBorder);
        delete(topBorder);
        delete(bottomBorder);
        
        for(std::vector<HudTextActor*>::iterator it = textCommands.begin(); it != textCommands.end(); ++it) {
            theWorld.Remove(*it);
            delete *it;
            
        }
        
    }
    
    bool HandleMessage(Message* m) {
        
        if (m->GetMessageName() == "AButton") {
            Dialog* dialog2 = new BlueDialog(Vec2i(120,120), 14);
            WindowManager::GetWindowManager()->PushDialog(dialog2);
        }
        else if (m->GetMessageName()=="BButton") {
            WindowManager::GetWindowManager()->PopDialog();
        }
        
        Dialog::HandleMessage(m);
        return true;
    }
    
private:
    Vec2i topLeft;
    Vec2i position;
    Vec2i size;
    int layer;
    
    HUDActor* background;
    HUDActor* leftBorder;
    HUDActor* rightBorder;
    HUDActor* topBorder;
    HUDActor* bottomBorder;
    vector<HudTextActor*> textCommands;
};

class BaseDialog : public Dialog {
public:
    BaseDialog(Cursor* cursor) {
        this->cursor = cursor;
        
        this->cameraMover = new CameraMessageListener();
        /* theSwitchboard.SubscribeTo(this->cameraMover,  "AButton");
         theSwitchboard.SubscribeTo(this->cameraMover,  "Up");
         theSwitchboard.SubscribeTo(this->cameraMover,  "Down");
         theSwitchboard.SubscribeTo(this->cameraMover,  "Left");
         theSwitchboard.SubscribeTo(this->cameraMover,  "Right"); */
    }
    
    bool HandleMessage(Message* m) {
        this->cursor->ReceiveMessage(m);
        this->cameraMover->ReceiveMessage(m);
        
        if (m->GetMessageName() == "AButton") {
            Dialog* dialog2 = new BlueDialog(Vec2i(120,120), 14);
            WindowManager::GetWindowManager()->PushDialog(dialog2);
            
        }
        return true;
    }
private:
    CameraMessageListener* cameraMover;
    Cursor* cursor;
};

static Board* board;


float cost(Vec2i from, Vec2i to) {
    return board->cost(from,to);
}


void get_neighbors(vector<Vec2i>& neighbors, Vec2i root) {
    if (root.X > 0) {
        neighbors.push_back(Vec2i(root.X-1, root.Y));
    }
    if (root.X <14) {
        neighbors.push_back(Vec2i(root.X+1, root.Y));
    }
    
    if (root.Y > 0) {
        neighbors.push_back(Vec2i(root.X, root.Y-1));
    }
    if (root.Y <14) {
        neighbors.push_back(Vec2i(root.X, root.Y+1));
    }
}

void dijstrka(float** distances, Vec2i size, int x, int y) {
    
    //float distances[15][15];
    Vec2i** previous;
    previous = new Vec2i*[size.X];
    for (int i=0; i<size.X; i++) {
        previous[i] = new Vec2i[size.Y];
    }
    
    vector<Vec2i> nodes;
    float MAX = 10000.0;
    for (int i=0; i<size.X; i++) {
        for (int j=0;j<size.Y;j++) {
            distances[i][j] = MAX;
            nodes.push_back(Vec2i(i,j));
        }
    }
    
    distances[x][y] = 0.0f;
    
    while(nodes.size() > 0) {
        
        Vec2i minNode;
        float minCost = MAX;
        
        for(std::vector<Vec2i>::iterator it = nodes.begin(); it != nodes.end(); ++it) {
            if (distances[it->X][it->Y] < minCost) {
                minCost = distances[it->X][it->Y];
                minNode = *it;
            }
        }
        
        if (minCost == MAX) {
            break;
        }
        
        for(std::vector<Vec2i>::iterator it = nodes.begin(); it != nodes.end(); ++it) {
            if (it->X == minNode.X && it->Y == minNode.Y) {
                nodes.erase(it);
                break;
            }
        }
        
        vector<Vec2i> neighbors;
        get_neighbors(neighbors, minNode);
        for(std::vector<Vec2i>::iterator it = neighbors.begin(); it != neighbors.end(); ++it) {
            float alt = distances[minNode.X][minNode.Y] + cost(minNode, *it);
            if (alt < distances[it->X][it->Y]) {
                distances[it->X][it->Y] = alt;
                previous[it->X][it->Y].X = minNode.X;
                previous[it->X][it->Y].Y = minNode.Y;
                
                
            }
        }
        //Find Smallest Cost
    }
    
    for(int i = 0; i < size.Y; ++i) {
        delete [] previous[i];
    }
    delete [] previous;
    
}

void CameraMessageListener::ReceiveMessage(Message* m) {
    
    //theWorld.GetConsole()->WriteToOutput();
    time_t t = time(0);
    struct tm* now = localtime(&t);
    sysLog.Log(IntToString(now->tm_hour) + ":" + IntToString(now->tm_min)
               + ":" + IntToString(now->tm_sec) + ": Message Received " + m->GetMessageName());
    
    Vector2 current_position = theCamera.GetPosition();
    Vector2 new_position(current_position.X, current_position.Y);
    if (m->GetMessageName().compare("Up")==0) {
        new_position.Y+=0.5f;
    }
    else if (m->GetMessageName().compare("Down")==0) {
        new_position.Y-=0.5f;
    }
    else if (m->GetMessageName().compare("Right")==0) {
        new_position.X+=0.5f;
    }
    else if (m->GetMessageName().compare("Left")==0) {
        new_position.X-=0.5f;
    }
    sysLog.Log("Camera Set To " + FloatToString(new_position.X)  + " " + FloatToString(new_position.Y));
    Vector3 new_position3 = Vector3(new_position.X, new_position.Y, 0);
    theCamera.MoveTo(new_position3, 0.15);
    
    /*
     if (!this->added) {
     theWorld.Add(this->t);
     this->added = true;
     Vector2 position = Vector2(10.0f, 0.0f);
     theCamera.MoveTo(position, 3);
     } else {
     Vector2 position = Vector2(0.0f, 0.0f);
     theCamera.MoveTo(position, 3, false );
     theWorld.Remove(this->t);
     this->added = false;
     }*/
}

Actor* build_particle() {
    ParticleActor* pa = new ParticleActor();
    pa->SetColor(1.0f, 1.0f, 1.0f);  //Sets the initial color of the particles.
    // Since the image file we'll be using already
    // has a color, we set this to pure white.
	
	pa->SetSize(Vector2(0.1f, 0.1f)); //The size of each particle, in GL units
	pa->SetSprite("Resources/Images/Test.png"); //The image file we want to use (otherwise
    // it'll just be colored squares).
	pa->SetMaxParticles(2500); //The maximum number of particles this system will ever handle.
	pa->SetParticlesPerSecond(20.0f); //Emission Rate
	pa->SetParticleLifetime(3.0f); //How long each particles lasts before disappearing
	pa->SetSpread(MathUtil::Pi/2.0f); //The angle in radians at which particles will be emitted.
	pa->SetEndScale(1.0f); //If you want the particles to change size over their lifetimes
	Color endColor(1.0f, 1.0f, 1.0f, 0.0f);
	pa->SetEndColor(endColor); //Our particles disappear over time
	pa->SetEndScale(2.0f);
	pa->SetSpeedRange(3.0f, 8.0f); //The minimum and maximum range of speeds (so you can have
    // some variation).
	pa->SetGravity(Vector2::Zero); //You can pull the particles in a particular direction (default is
    // downwards, so zero it out if you need to).
    
    return pa;
    
}

int main(int argc, char* argv[])
{
	// get things going
	//  optional parameters:
	//		int windowWidth			default: 1024
	//		int windowHeight		default: 768
	//		std::string windowName	default: "Angel Engine"
	//		bool antiAliasing		default: false
	//		bool fullScreen			default: false
    
    
    
    
	theWorld.Initialize(1024,768, "Warsong Redux", true, false, true);
    
    board = new Board(Vec2i(30,25));
    
    theSpatialGraph;
    
    std::vector<Vec3ui> modes = theWorld.GetVideoModes();
    for(std::vector<Vec3ui>::iterator it = modes.begin(); it != modes.end(); ++it) {
        std::ostringstream stringbuilder;
        stringbuilder<<it->X<<" "<<it->Y<<" "<<it->Z<<endl  ;
        theWorld.GetConsole()->WriteToOutput(stringbuilder.str());
    }
    
    //theWorld.Add(build_particle(), 0 );
    
	theWorld.GetConsole()->WriteToOutput("here");
	//adds the default grid so you can more easily place Actors
    
	//yay for magic numbers! (default parameters of the grid)
	Color _lineColor = Color(.76f, .83f, 1.0f);
	Color _axisColor = Color(1.0f, .41f, .6f);
    Vector2 _minCoord = Vector2(-20.0f, -20.0f);
	Vector2 _maxCoord = Vector2(20.0f, 20.0f);
    GridActor* grid = new GridActor(_lineColor, _axisColor, 1.0f, _minCoord, _maxCoord );
    
	theWorld.Add(grid, -1);
    
	
    Vector3 position = Vector3(0.0f, 0.0f, 5.0f);
    Camera::GetInstance().SetPosition(position);
    
    
    
    
    Actor* bottomBorder = new HUDActor();
    bottomBorder->SetPosition(512, 708);
    bottomBorder->SetSize(1024,120);
    bottomBorder->SetSprite("Resources/Images/bottom_border.png");
    bottomBorder->SetLayer(15);
    theWorld.Add(bottomBorder);
    
    Soldier* s = new Soldier();
    Soldier* s2 = new Soldier(1,1);
    Soldier* s3 = new Soldier(0,1);
    Soldier* s4 = new Soldier(3,3);
    
    Cursor* cursor = new Cursor();
    WindowManager* windowManager = WindowManager::GetWindowManager();
    windowManager->PushDialog(new BaseDialog(cursor));
    
    Dialog* dialog = new BlueDialog(Vec2i(100,100), 10);
    windowManager->PushDialog(dialog);
    
    RegisterFont("Resources/Fonts/Inconsolata.otf", 24, "Inconsolata");
    
    float** distances;
    distances = new float*[board->GetBoardSize().X];
    for (int i=0; i<board->GetBoardSize().X;i++) {
        distances[i] = new float[board->GetBoardSize().Y];
    }
    
    dijstrka(distances,board->GetBoardSize(), 3,3);
    for(int i=0;i<board->GetBoardSize().X;i++){
        for(int j=0;j<board->GetBoardSize().Y; j++){
            if (distances[i][j]>4.0) {
                Actor* g = new Actor();
                g->SetPosition(i, j);
                
                g->SetColor(0.3, 0.3, 0.3);
                g->SetAlpha(0.8f);
                theWorld.Add(g,5);
                
            }
            
        }
    }
    
    //theWorld.Add(board, 1);
    
    
    
	//YOUR GAME SETUP CODE HERE
    //InputManager::GetInstance().IsKeyDown(<#int keyVal#>)
	
	// do all your setup first, because this function won't return until you're exiting
	theWorld.StartGame();
    
    
	// any cleanup can go here
	theWorld.Destroy();
	
	return 0;
}
