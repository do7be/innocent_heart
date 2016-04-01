#include<gl/glut.h>
#include<time.h>
#include<stdio.h>
#include<unistd.h>
#include<math.h>
#include<string.h>
#include<gl/glpng.h>
#include<stdlib.h>

void Display(void);
void Reshape(int w,int h);
void Timer(int);
void init(void);
void PutMap(char map,int x,int y,int n);//マップの表示
void PutChar(void);//キャラの表示
void PutTeki(int n,int what);//敵キャラ,アイテムの表示
void PutMagic(int what);//魔法の残り使用数表示
void PutLife(void);//ボスの残りライフ表示
void PutSprite(int x,int y,int width,int height);//画像貼り付け
void PutSprite_RE(int x,int y,int width,int height);//画像反転貼り付け
void Attack(int what);//攻撃処理
void Attack_teki(int n);//敵の攻撃処理
void Attack_boss(int n);//ボスの攻撃処理
void Anime(int what,int teki_n,int boss_n);//攻撃などのアニメ
void Keyboard(char key,int x,int y);//キー処理
void KeyboardUp(char key,int x,int y);//キー処理
void SpecialKey(int key,int x,int y);//キー処理
void SpecialUpKey(int key,int x,int y);//キーUP処理
void Teki_hantei(void);//敵との衝突や敵の攻撃、アイテムの判定などを処理する
void Boss_hantei(void);//ボスとの衝突や攻撃を処理する
void Kabe_hantei(int who,int n);//壁判定
void Die(void);//死亡後の処理
void Stage_Change(void);//ステージを変える
void Game_Clear(void);//ゲームクリア後の処理

#define WindowWidth 640//ウィンドウの大きさ
#define WindowHeight 480
#define MapWidth 101
#define MapHeight 16
#define Tex_NUM 3//マップイメージ数
#define Scene_NUM 14//シーンのイメージ数
#define NUM_IMAGE 4//数字画像の数
#define Char_NUM 3
#define Teki_NUM 25
#define Attack_NUM 4//攻撃の種類
#define Boss_Attack_NUM 4//ボスの攻撃の種類
#define Char_Size 32//イメージサイズ
#define Boss_Size 64//ボスサイズ
#define Life_Size 16//ボスのライフサイズ
#define Step 4
#define JumpHeight 44//ジャンプ力(初速度)

struct attack_anime//攻撃の属性
{
    int x,y,t;//tは時間
    int flag;//攻撃中かどうか
    int direction;
    int limit;

    pngInfo info[3];
    GLuint texture[3];
};

struct teki_status//敵の情報
{
    int x,y,cursor,t,walk_flag;
    int flag,out_flag;//生きているかどうか,画面の外にいたかどうか
    int direction;

    pngInfo info[2];
    GLuint texture[2];
};

struct teki_attack_anime//敵の攻撃
{
    int x,y,t;
    float x_move,y_move;//moveは移動量
    float x_sum,y_sum;
    int flag;
    int direction;

    pngInfo info[3];
    GLuint texture[3];
};

struct item_status//アイテムの情報
{
    int x,y,kind;//kindはアイテムの種類
    int flag,out_flag,appear_flag;
    //flagは表示するかしないか
    //out_flagは画面外かどうか
    //appear_flagは存在しているかどうか
    pngInfo info[1];
    GLuint texture[1];
};

struct boss_status//ボスの情報
{
    int x,y,cursor,t,walk_flag;
    int flag;//生きているかどうか
    int direction;
    int life,nodamage_t;

    pngInfo info[2];
    GLuint texture[2];
};

struct boss_attack_anime//ボスの攻撃
{
    int x[3],y[3],t;
    int x_move[3],y_move[3];
    int flag;
    int direction;

    pngInfo info[3];
    GLuint texture[3];
};

//+++++++++++++++++++++++++++++++++++++テクスチャ+++++++++++++++++++++++++++++++++
pngInfo map_info[Tex_NUM+1],scene_info[Scene_NUM],Char_info[Char_NUM],num_info[NUM_IMAGE],item_info[Attack_NUM-1],life_info;
GLuint  map_texture[Tex_NUM],scene_texture[Scene_NUM],Char_texture[Char_NUM],num_texture[NUM_IMAGE],item_texture[Attack_NUM-1],life_texture;

int walk_flag=0,walk_count=0,right_end_flag=0;
int x_char,y_char,y_tempchar=0;
int cursor_flag=0,cursor_key=0,direction=2;//cursor=0:なし,1:上,2:右,3:左,4:下をカーソル入力した
int jump_flag=0,jump_key;
int stage=0,stage_start=0;//シーン用フラグ
float jump_t;
int z_key,a_key,s_key,d_key,fall_flag=0,pause_flag=1,pause_key=0,die_t;
int clear_flag,clear_scene,z_up,change_flag;

//++++++++++++攻撃の構造体設定+++++++++++++++
static struct attack_anime attack[Attack_NUM];//攻撃の種類数
static struct teki_status teki[Teki_NUM];//敵の数
static struct teki_attack_anime teki_attack[Teki_NUM];//敵の攻撃
static struct item_status item[Teki_NUM];//アイテムの数=敵の数
static struct boss_status boss;//ボス情報
static struct boss_attack_anime boss_attack[Boss_Attack_NUM];//ボスの攻撃

char map[MapHeight][MapWidth+1];

//
//Program start
//
int main(int argc, char **argv)
{
    srandom((unsigned)time(NULL));

    int width,height;
    int i,j;
    char file_name[50];

    glutInit(&argc, argv);
    glutInitWindowSize(WindowWidth, WindowHeight);
    glutCreateWindow("Innocent Heart");
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_ALPHA);
    glClearColor(0.0, 0.0, 1.0, 1.0);

    width=glutGet(GLUT_SCREEN_WIDTH);//ディスプレイの幅
    height=glutGet(GLUT_SCREEN_HEIGHT);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

    //+++++++++++ロード画面の画像読み込み++++++++++++++++++++++++++++
    for(i=0;i<Scene_NUM;i++)
    {
        sprintf(file_name,"image/scene_%d.png",i+1);
        scene_texture[i] = pngBind(file_name, PNG_NOMIPMAP, PNG_ALPHA, &scene_info[i], GL_CLAMP, GL_NEAREST, GL_NEAREST);
    }

    //+++++++++++キャラ画像読み込み+++++++++++++++++++++++++++++
    for(i=0;i<Char_NUM;i++)
    {
        sprintf(file_name,"image/char_%d.png",i+1);
        Char_texture[i] = pngBind(file_name, PNG_NOMIPMAP, PNG_ALPHA, &Char_info[i], GL_CLAMP, GL_NEAREST, GL_NEAREST);
    }

    //++++++++++++敵の画像読み込み+++++++++++++++
    for(j=0;j<Teki_NUM;j++)
    {
        for(i=0;i<2;i++)
        {
            sprintf(file_name,"image/teki%d_%d.png",j%4+1,i+1);
            teki[j].texture[i] = pngBind(file_name, PNG_NOMIPMAP, PNG_ALPHA, &teki[j].info[i], GL_CLAMP, GL_NEAREST, GL_NEAREST);
        }
    }

    //++++++++++++敵の攻撃の画像読み込み+++++++++++++++
    for(j=0;j<Teki_NUM;j++)
    {
        for(i=0;i<3;i++)
        {
            if(j%4==2)//デーモンの魔法
                sprintf(file_name,"image/teki_attack_%d.png",i+1);
            else//ウィザードの魔法
                sprintf(file_name,"image/teki_attack_wind_%d.png",i+1);
            teki_attack[j].texture[i] = pngBind(file_name, PNG_NOMIPMAP, PNG_ALPHA, &teki_attack[j].info[i], GL_CLAMP, GL_NEAREST, GL_NEAREST);
        }
    }
    //++++++++++++ボスの攻撃の画像読み込み+++++++++++++++
    for(j=0;j<Boss_Attack_NUM;j++)
    {
        for(i=0;i<3;i++)
        {
            switch(j)
            {
                case 0 : sprintf(file_name,"image/boss_attack_darkthunder_%d.png",i+1);break;
                case 1 : sprintf(file_name,"image/boss_attack_bluefire_%d.png",i+1);break;
                case 2 : sprintf(file_name,"image/boss_attack_waterballoon_%d.png",i+1);break;
                case 3 : sprintf(file_name,"image/boss_attack_meteor_%d.png",i+1);break;
            };
            boss_attack[j].texture[i] = pngBind(file_name, PNG_NOMIPMAP, PNG_ALPHA, &boss_attack[j].info[i], GL_CLAMP, GL_NEAREST, GL_NEAREST);
        }
    }

    //++++++++++++アイテムの画像読み込み+++++++++++++++
    for(j=0;j<Teki_NUM;j++)
    {
        for(i=0;i<1;i++)
        {
            item[j].kind=3*(random()/(double)(RAND_MAX+1.0));//アイテムの種類をランダムで選択
            sprintf(file_name,"image/item_%d.png",item[j].kind+1);
            item[j].texture[i] = pngBind(file_name, PNG_NOMIPMAP, PNG_ALPHA, &item[j].info[i], GL_CLAMP, GL_NEAREST, GL_NEAREST);
        }
    }

    //++++++++++++残り魔法使用回数の画像読み込み+++++++++++++++
    for(i=0;i<Attack_NUM-1;i++)//3種類
    {
        sprintf(file_name,"image/item_%d.png",i+1);
        item_texture[i] = pngBind(file_name, PNG_NOMIPMAP, PNG_ALPHA, &item_info[i], GL_CLAMP, GL_NEAREST, GL_NEAREST);
    }

    //++++++++++++ボスの残りライフの画像読み込み+++++++++++++++
    sprintf(file_name,"image/life.png");
    life_texture = pngBind(file_name, PNG_NOMIPMAP, PNG_ALPHA, &life_info, GL_CLAMP, GL_NEAREST, GL_NEAREST);

    //++++++++++++数字画像読み込み+++++++++++++++
    for(i=0;i<NUM_IMAGE;i++)
    {
        sprintf(file_name,"image/num_%d.png",i);
        num_texture[i] = pngBind(file_name, PNG_NOMIPMAP, PNG_ALPHA, &num_info[i], GL_CLAMP, GL_NEAREST, GL_NEAREST);
    }

    //++++++++++++攻撃の画像読み込み+++++++++++++++
    for(j=0;j<Attack_NUM;j++)
    {
        for(i=0;i<3;i++)
        {
            switch(j)
            {
                case 0 : sprintf(file_name,"image/sword_%d.png",i+1);break;
                case 1 : sprintf(file_name,"image/fire_%d.png",i+1);break;
                case 2 : sprintf(file_name,"image/ice_%d.png",i+1);break;
                case 3 : sprintf(file_name,"image/thunder_%d.png",i+1);break;
            };
            attack[j].texture[i] = pngBind(file_name, PNG_NOMIPMAP, PNG_ALPHA, &attack[j].info[i], GL_CLAMP, GL_NEAREST, GL_NEAREST);
        }
        attack[j].limit=3;
    }
    //++++++++++++ボスの画像読み込み+++++++++++++++
    for(i=0;i<2;i++)
    {
        sprintf(file_name,"image/boss_%d.png",i+1);
        boss.texture[i] = pngBind(file_name, PNG_NOMIPMAP, PNG_ALPHA, &boss.info[i], GL_CLAMP, GL_NEAREST, GL_NEAREST);
    }

    init();//初期化

    glEnable(GL_BLEND);//テクスチャのアルファチャンネル有効
    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
    glTexEnvf(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);

    glEnable(GL_CULL_FACE);

    glutDisplayFunc(Display);//Display定義
    glutReshapeFunc(Reshape);//Reshape定義
    glutTimerFunc(500,Timer,0);//0.5秒ずつ呼び出し

    glutKeyboardFunc(Keyboard);
    glutKeyboardUpFunc(KeyboardUp);
    glutSpecialFunc(SpecialKey);
    glutSpecialUpFunc(SpecialUpKey);

    glutMainLoop();

    return(0);
}

void init(void)//初期化 コンテニュー時などで呼ぶ
{
    int i;

    cursor_flag=0;cursor_key=0;direction=2;
    jump_flag=0;fall_flag=0;jump_t=0;
    if(boss.flag==1)//復活後のキャラクターの位置(ボス戦)
    {
        x_char=Char_Size*75;y_char=Char_Size*12;
    }
    else
    {
        x_char=Char_Size*2;y_char=Char_Size*12;
        boss.flag=0;
    }
    right_end_flag=0;//右端のカメラ固定を解除
    pause_flag=1;
    die_t=0;//死亡時間0=死んでいない

    for(i=0;i<Attack_NUM;i++)
    {
        attack[i].flag=0;
        attack[i].t=0;
    }
    for(i=0;i<Teki_NUM;i++)
    {
        teki[i].flag=1;
        teki[i].out_flag=0;
        teki[i].direction=2;//初期設定では敵は右を向いている
        teki[i].cursor=0;
        item[i].flag=0;//アイテムを消す
        item[i].out_flag=0;
        item[i].appear_flag=0;
        teki_attack[i].flag=0;//敵の攻撃を消す
        teki_attack[i].t=0;
    }
    boss.direction=3;boss.cursor=0;
    boss.nodamage_t=0;
    for(i=0;i<Boss_Attack_NUM;i++)
    {
        boss_attack[i].flag=0;//ボスの攻撃を消す
        boss_attack[i].t=0;
    }

    for(i=0;i<Attack_NUM;i++)
        attack[i].limit=3;//魔法使用回数初期化
    clear_flag=0;clear_scene=4;z_up=1;//ゲームクリア用
    change_flag=0;
}

//
//ウィンドウの表示内容を更新する関数
//
void Display(void)
{
    int i,j,n=0;

    if(stage==0 || stage_start==0)//タイトル画面かステージ開始時かクリア後
    {
        return;
    }

    glClear(GL_COLOR_BUFFER_BIT);

    if(x_char > WindowWidth/2)//真ん中より右に行ったとき
    {
        n=(x_char-WindowWidth/2)/Char_Size;
        if(x_char >= Char_Size*(MapWidth-11) || right_end_flag==1)//右端
        {
            n=MapWidth-21;//画面固定
            if(x_char==Char_Size*(MapWidth-2) && stage==1)//次のステージへ
            {
                stage=2;
                Stage_Change();
                return;
            }
        }
    }

    for(i=0;i<15;i++)//マップ表示
    {
        for(j=0;j<=20;j++)
        {
            PutMap(map[i][j+n],j,i*map_info->Height,n);
        }
    }

    //+++++++++++各キャラクターや攻撃の処理+++++++++++++++++++
    PutChar();
    if(boss.flag==1)//ボス戦
    {
        for(i=0;i<Boss_Attack_NUM;i++)
        {
            if(boss_attack[i].flag==1)//ボスの攻撃
                Attack_boss(i);
        }
        PutTeki(0,2);//ボスの表示
        PutLife();//ボスのライフ表示
    }
    else
    {
        for(i=0;i<Teki_NUM;i++)
        {
            if(teki_attack[i].flag==1)//敵の攻撃
                Attack_teki(i);
            if(item[i].flag==1)
                PutTeki(i,1);//アイテムの表示
            if(teki[i].flag==1)
                PutTeki(i,0);//敵の表示
        }
    }
    for(i=0;i<Attack_NUM;i++)
    {
        if(i!=0)//魔法の残り使用回数を表示
            PutMagic(i);
        if(attack[i].flag==1)
            Attack(i);
    }

    glFlush();
    glutSwapBuffers();
}


void Timer(int value)
{
    glutTimerFunc(24,Timer,0);

    if(pause_flag==0)//ゲーム実行中
    {
        Kabe_hantei(0,0);//charの壁判定

        if(right_end_flag==1)//ボス戦
            Boss_hantei();
        else if(boss.flag==0)
            Teki_hantei();//敵との衝突や攻撃、アイテムの判定の処理

        if(y_char+Char_Size>=WindowHeight-Char_Size)//下まで落ちたとき
        {
            //+++++++++++++++++++落ちた++++++++++++++++++++++++++
            Stage_Change();
        }

        Display();
        walk_count++;
        if(walk_count==20)
        {
            walk_count=0;
            walk_flag=walk_flag^1;//歩行絵切り替え
        }

        if(die_t!=0)//死んだとき
            Die();
    }
    else if(stage==0 && change_flag==0)//タイトル画面
    {
        Stage_Change();
        change_flag=1;
    }
    else if(clear_flag==1)//クリア後の処理
    {
        Game_Clear();
    }
}

void Reshape(int w,int h)
{
    glViewport(0,0,w,h);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluOrtho2D(0,w,0,h);
    glScaled(1,-1,1);
    glTranslated(0,-h,0);
    glutReshapeWindow(WindowWidth,WindowHeight);
}
void PutMap(char map,int x,int y,int n)//マップ表示
{
    int tex;

    switch(map)
    {
        case 'A' : tex=0; break;
        case 'B' : tex=1; break;
        case 'C' : tex=2; break;
        default : printf("ERROR\n");exit(1);
    };

    if(x_char >= Char_Size*(MapWidth-11) || right_end_flag==1)//画面右端
        x*=map_info->Width;//右端まで行くと引き返せなくなる
    else if(x_char>=WindowWidth/2)//スクロール
        x=x*Char_Size+WindowWidth/2-x_char+n*Char_Size;
    else
        x*=map_info->Width;

    glBindTexture(GL_TEXTURE_2D, map_texture[tex]);

    PutSprite(x,y,Char_Size,Char_Size);
}
void PutChar(void)//キャラ表示
{
    int x;
    int tex;
    if(x_char >= Char_Size*(MapWidth-11) || right_end_flag==1)//画面右端
    {
        x=x_char-Char_Size*(MapWidth-21);
        if(stage==2 && right_end_flag==0)//stage2のみボス戦でカメラ固定のため
        {
            right_end_flag=1;//右端まで行くと引き返せなくなる
            boss.flag=1;//ボス出現
            map[11][78]='C';//ボス戦で壁生成
            map[12][78]='C';
            map[11][99]='C';
        }
    }
    else if(x_char>=WindowWidth/2)//スクロール
        x=WindowWidth/2;
    else
        x=x_char;

    tex=walk_flag;//表示画像(歩いている)
    if(die_t!=0)//死んでいるかどうか
        tex=2;//死んだ画像

    glBindTexture(GL_TEXTURE_2D, Char_texture[tex]);

    if(direction==2)
        PutSprite_RE(x,y_char,Char_Size,Char_Size);//画像反転
    else
        PutSprite(x,y_char,Char_Size,Char_Size);//画像貼り付け
}
void PutTeki(int n,int what)//敵表示,アイテム表示,ボス表示
{
    int x,y,size;
    int direction;
    GLuint texture;

    switch(what)
    {
        case 0:x=teki[n].x;y=teki[n].y;texture=teki[n].texture[teki[n].walk_flag];size=Char_Size;direction=teki[n].direction;break;
        case 1:x=item[n].x;y=item[n].y;texture=item[n].texture[0];size=Char_Size;break;
        case 2:x=boss.x;y=boss.y;texture=boss.texture[boss.walk_flag];size=Boss_Size;direction=boss.direction;
                if(boss.nodamage_t%10>7)//ボスに攻撃した後の無敵時間ではボスが点滅する
                    return;
                break;
    };

    if(x_char >= Char_Size*(MapWidth-11) || right_end_flag==1)
        x=x-Char_Size*(MapWidth-21);
    else if(x_char>=WindowWidth/2)
        x=x-x_char+WindowWidth/2;

    glBindTexture(GL_TEXTURE_2D, texture);

    if(direction==2 && what!=1)//敵とボスは方向転換あり
        PutSprite_RE(x,y,size,size);//画像反転
    else
        PutSprite(x,y,size,size);
}
void PutMagic(int n)//残り魔法使用回数表示
{
    int x,y;

    x=0;
    y=Char_Size*(n-1);

    glBindTexture(GL_TEXTURE_2D, item_texture[n-1]);//残り使用回数の背景
    PutSprite(x,y,Char_Size,Char_Size);
    glBindTexture(GL_TEXTURE_2D, num_texture[attack[n].limit]);//残り使用回数
    PutSprite(x,y,Char_Size,Char_Size);
}
void PutLife()//ボスの残りライフ表示
{
    int x,y,i;

    x=WindowWidth;
    y=0;

    for(i=1;i<=boss.life;i++)//残りライフの分ハートを表示
    {
        glBindTexture(GL_TEXTURE_2D, life_texture);//残り使用回数の背景
        if(i<=10)
            PutSprite(x-Life_Size*i,y,Life_Size,Life_Size);//1段目
        else
            PutSprite(x-Life_Size*(i-10),y+Life_Size,Life_Size,Life_Size);//2段目
    }
}
void PutSprite(int x,int y,int width,int height)//画像貼り付け
{
    //画像貼り付け
    glPushMatrix();
    glEnable(GL_TEXTURE_2D);

        glBegin(GL_QUADS);
        glTexCoord2f(1, 1); glVertex2i(x + width , y + height);
        glTexCoord2f(1, 0); glVertex2i(x + width , y);
        glTexCoord2f(0, 0); glVertex2i(x , y);
        glTexCoord2f(0, 1); glVertex2i(x , y + height);
        glEnd();
    glDisable(GL_TEXTURE_2D);
    glPopMatrix();
}
void PutSprite_RE(int x,int y,int width,int height)//反転して画像貼り付け
{
    glPushMatrix();
    glEnable(GL_TEXTURE_2D);

        glBegin(GL_QUADS);
        //+++++++++++++++++++画像の横方向に反転させている+++++++++++++++
        glTexCoord2f(0, 1); glVertex2i(x + width , y + height);
        glTexCoord2f(0, 0); glVertex2i(x + width , y);
        glTexCoord2f(1, 0); glVertex2i(x , y);
        glTexCoord2f(1, 1); glVertex2i(x , y + height);
        glEnd();
    glDisable(GL_TEXTURE_2D);
    glPopMatrix();
}

void Attack(int what)//攻撃処理
{
    if(what==0)//sword
    {
        attack[what].direction=direction;
        if(attack[what].direction==2)
            attack[what].x=x_char+Char_Size;
        else
            attack[what].x=x_char-Char_Size;
        attack[what].y=y_char;
        Anime(what,0,0);
        attack[what].t++;

        if(attack[what].t>=10)
        {
            attack[what].t=0;
            attack[what].flag=0;
        }
    }
    else if(what==1 || what==2)//fire,ice
    {
        if(attack[what].t==0)//撃った直後
        {
            attack[what].y=y_char;
            if(direction==2)
            {
                attack[what].x=x_char+Char_Size;
                attack[what].direction=2;
            }
            else
            {
                attack[what].x=x_char-Char_Size;
                attack[what].direction=3;
            }
            if(what==3)//iceの場合上昇する
                attack[what].y=y_char-Char_Size;
        }
        else
        {
            if(attack[what].direction==2)
                attack[what].x+=Step*2;
            else
                attack[what].x-=Step*2;
            if(what==2)//ice
                attack[what].y-=Step*2;
        }

        Anime(what,0,0);
        attack[what].t++;

        if(attack[what].t>=30)
        {
            attack[what].t=0;
            attack[what].flag=0;
        }
    }
    else if(what==3)//thunder
    {
        if(attack[what].t==0)//撃った直後
        {
            attack[what].y=0;
            attack[what].x=x_char;
            if(direction==2)
            {
                attack[what].direction=2;
            }
            else
            {
                attack[what].direction=3;
            }
        }
        else if(attack[what].t>=5)
        {
            attack[what].y+=Step*3;
        }

        Anime(what,0,0);
        attack[what].t++;

        if(attack[what].t>=45)
        {
            attack[what].t=0;
            attack[what].flag=0;
        }
    }
}
void Attack_teki(int n)//敵の攻撃処理
{
    float temp;


    if(teki_attack[n].t==0)//撃った直後
    {
        teki_attack[n].y_sum=teki[n].y;

        if(teki[n].direction==2)//右向き
        {
            teki_attack[n].x_sum=teki[n].x+Char_Size;
            teki_attack[n].direction=2;
        }
        else//左向き
        {
            teki_attack[n].x_sum=teki[n].x-Char_Size;
            teki_attack[n].direction=3;
        }

        if(n%4==2)//デーモン
        {
            temp=atan((float)(y_char-teki_attack[n].y_sum)/(float)(x_char-teki_attack[n].x_sum));//角度を求める
            teki_attack[n].x_move=((float)(x_char-teki_attack[n].x_sum))/50;//xは50回移動でキャラに到達
            teki_attack[n].y_move=tan(temp)*teki_attack[n].x_move;//yの移動量
        }
        else if(n%4==3)//ウィザード
        {
            if(teki[n].direction==2)
                teki_attack[n].x_move=Step*2;
            else
                teki_attack[n].x_move=-1*Step*2;
            teki_attack[n].y_move=0;
        }
    }
    else
    {
        //移動後のx,yを計算
            teki_attack[n].x_sum+=teki_attack[n].x_move;
            teki_attack[n].y_sum+=teki_attack[n].y_move;
    }
    //x,yの位置を代入
    teki_attack[n].x=(int)teki_attack[n].x_sum;
    teki_attack[n].y=(int)teki_attack[n].y_sum;

    Anime(-1,n,0);//アニメを表示
    teki_attack[n].t++;

    if(teki_attack[n].t>=60)
    {
        teki_attack[n].t=0;
        teki_attack[n].flag=0;
    }
}
void Attack_boss(int n)//ボスの攻撃処理
{
    int i;

    if(boss_attack[n].t==0)//撃った直後
    {
        if(n==0)//ダークサンダーの場合
        {
            boss_attack[n].x[0]=x_char;boss_attack[n].y[0]=0;//出現位置
            boss_attack[n].x[1]=x_char-Char_Size;boss_attack[n].y[1]=0;
            boss_attack[n].x[2]=x_char+Char_Size;boss_attack[n].y[2]=0;
            boss_attack[n].direction=boss.direction;
            boss_attack[n].x_move[0]=0;boss_attack[n].y_move[0]=0;//移動量
            boss_attack[n].x_move[1]=0;boss_attack[n].y_move[1]=0;
            boss_attack[n].x_move[2]=0;boss_attack[n].y_move[2]=0;
        }
        else if(n==1 || n==2)//ブルーファイアとウォーターバルーンの場合
        {
            boss_attack[n].y[0]=boss.y+Char_Size;//出現位置
            boss_attack[n].y[1]=boss.y+Char_Size;
            boss_attack[n].y[2]=boss.y+Char_Size;

            if(boss.direction==2)//右向き
            {
                boss_attack[n].x[0]=boss.x+Boss_Size;//出現位置
                boss_attack[n].x[1]=boss.x+Boss_Size;
                boss_attack[n].x[2]=boss.x+Boss_Size;
                boss_attack[n].direction=2;
                boss_attack[n].x_move[0]=Step;//移動量
                boss_attack[n].x_move[1]=Step;
                boss_attack[n].x_move[2]=Step;
            }
            else//左向き
            {
                boss_attack[n].x[0]=boss.x-Char_Size;//出現位置
                boss_attack[n].x[1]=boss.x-Char_Size;
                boss_attack[n].x[2]=boss.x-Char_Size;
                boss_attack[n].direction=3;
                boss_attack[n].x_move[0]=-1*Step;//移動量
                boss_attack[n].x_move[1]=-1*Step;
                boss_attack[n].x_move[2]=-1*Step;
            }
            boss_attack[n].y_move[0]=0;
            boss_attack[n].y_move[1]=0;
            boss_attack[n].y_move[2]=0;
            if(n==2)//ウォーターバルーンの場合は1つ目の水玉のみ動き始める
            {
                boss_attack[n].x_move[1]=0;
                boss_attack[n].x_move[2]=0;
            }
        }
        else//メテオの場合
        {//出現位置はランダム
            boss_attack[n].x[0]=Char_Size*79 + Char_Size*(int)(20*(random()/(double)(RAND_MAX+1.0)));boss_attack[n].y[0]=0;
            boss_attack[n].x[1]=Char_Size*79 + Char_Size*(int)(20*(random()/(double)(RAND_MAX+1.0)));boss_attack[n].y[1]=0;
            boss_attack[n].x[2]=Char_Size*79 + Char_Size*(int)(20*(random()/(double)(RAND_MAX+1.0)));boss_attack[n].y[2]=0;
            boss_attack[n].direction=boss.direction;
            boss_attack[n].x_move[0]=0;boss_attack[n].y_move[0]=Step*4;//移動量
            boss_attack[n].x_move[1]=0;boss_attack[n].y_move[1]=Step*4;
            boss_attack[n].x_move[2]=0;boss_attack[n].y_move[2]=Step*4;
        }
    }
    else
    {
        if(boss_attack[n].t==6 && n==0)//ダークサンダー落下開始
        {
            boss_attack[n].y_move[0]=Step*3;
            boss_attack[n].y_move[1]=Step*3;
            boss_attack[n].y_move[2]=Step*3;
        }
        else if(boss_attack[n].t==20 && n==1)//ブルーファイア軌道変化
        {
            boss_attack[n].y_move[0]=0;//3方向に飛ぶ
            boss_attack[n].y_move[1]=-1*Step;
            boss_attack[n].y_move[2]=-1*Step;boss_attack[n].x_move[2]=0;
        }
        else if(n==2)//ウォーターバルーン軌道変化
        {
            switch(boss_attack[n].t)//時間差で2発目と3発目が動き出す
            {
                case 10 : boss_attack[n].x_move[1]=Step;if(boss_attack[n].direction==3)boss_attack[n].x_move[1]*=-1;break;
                case 20 : boss_attack[n].x_move[2]=Step;if(boss_attack[n].direction==3)boss_attack[n].x_move[2]*=-1;break;
            };
        }
        //移動後のx,yを計算
        boss_attack[n].x[0]+=boss_attack[n].x_move[0];boss_attack[n].y[0]+=boss_attack[n].y_move[0];
        boss_attack[n].x[1]+=boss_attack[n].x_move[1];boss_attack[n].y[1]+=boss_attack[n].y_move[1];
        boss_attack[n].x[2]+=boss_attack[n].x_move[2];boss_attack[n].y[2]+=boss_attack[n].y_move[2];
    }

    for(i=0;i<3;i++)
        Anime(-2,n,i);//アニメを表示
    boss_attack[n].t++;

    if(boss_attack[n].t>=90)
    {
        boss_attack[n].t=0;
        boss_attack[n].flag=0;
    }
}
void Anime(int what,int teki_n,int boss_n)//攻撃などのアニメ
{
    int x,y,tex,t_tmp,attack_direction;
    GLuint texture;

    if(x_char>=WindowWidth/2)//スクロールはじめの場所
        x=attack[what].x-x_char+WindowWidth/2;
    else
        x=attack[what].x;

    if(what>=0)//キャラの攻撃
    {
        if(x_char >= Char_Size*(MapWidth-11) || right_end_flag==1)//スクロール終わりの場所
        {
            if(what==0)//sword
            {
                x=x_char-Char_Size*(MapWidth-21);
                if(direction==2)
                    x+=Char_Size;
                else
                    x-=Char_Size;
            }
            else
            {
                x=attack[what].x-Char_Size*(MapWidth-21);
            }
        }
        if(what==0)//sword
        {
            if(attack[what].t<=2)
                tex=0;
            else if(attack[what].t<=5)
                tex=1;
            else if(attack[what].t<=9)
                tex=2;
        }
        else//ファイア、アイス、サンダー
        {
            if(what==3)//サンダー用
                t_tmp=attack[what].t%2+1;
            else//ファイアとアイス用
                t_tmp=attack[what].t%3;

            if(attack[what].t<5 && what==3)//サンダーの場合は最初だけ光った画像
                tex=0;
            else
                tex=t_tmp;
        }
    }
    else if(what==-1)//敵の攻撃
    {
        if(x_char>=WindowWidth/2)//スクロールはじめの場所
            x=teki_attack[teki_n].x-x_char+WindowWidth/2;
        else
            x=teki_attack[teki_n].x;
        t_tmp=teki_attack[teki_n].t%3;

        if(t_tmp==0)
            tex=0;
        else if(t_tmp==1)
            tex=1;
        else if(t_tmp==2)
            tex=2;
        if(x_char >= Char_Size*(MapWidth-11) || right_end_flag==1)//スクロール終わりの場所
            x=teki_attack[teki_n].x-Char_Size*(MapWidth-21);
    }
    else if(what==-2)//ボスの攻撃
    {
        if(x_char>=WindowWidth/2)//スクロールはじめの場所
            x=boss_attack[teki_n].x[boss_n]-x_char+WindowWidth/2;
        else
            x=boss_attack[teki_n].x[boss_n];

        if(teki_n==0)//ダークサンダー用
            t_tmp=boss_attack[teki_n].t%2+1;
        else//ブルーファイアとウォーターバルーンとメテオ用
            t_tmp=boss_attack[teki_n].t%3;

        if(boss_attack[teki_n].t<5 && teki_n==0)//ダークサンダーの場合は最初だけ光った画像
            tex=0;
        else if(t_tmp==0 && teki_n==1)//ブルーファイア用
            tex=0;
        else
            tex=t_tmp;
        if(x_char >= Char_Size*(MapWidth-11) || right_end_flag==1)//スクロール終わりの場所
            x=boss_attack[teki_n].x[boss_n]-Char_Size*(MapWidth-21);
    }

    if(what>=0)//キャラの攻撃のとき
    {
        attack_direction=attack[what].direction;
        texture=attack[what].texture[tex];
        y=attack[what].y;
    }
    else if(what==-1)//敵の攻撃のとき
    {
        attack_direction=teki_attack[teki_n].direction;
        texture=teki_attack[teki_n].texture[tex];
        y=teki_attack[teki_n].y;
    }
    else if(what==-2)//ボスの攻撃のとき
    {
        attack_direction=boss_attack[teki_n].direction;
        texture=boss_attack[teki_n].texture[tex];
        y=boss_attack[teki_n].y[boss_n];
    }

    glBindTexture(GL_TEXTURE_2D,texture);

    if(attack_direction==2)//右方向
    {
        PutSprite(x,y,Char_Size,Char_Size);
    }
    else//左方向
    {
        PutSprite_RE(x,y,Char_Size,Char_Size);//画像反転
    }
}
void Teki_hantei(void)//++++++++++++++++++敵との衝突や攻撃、アイテムの判定など+++++++++++
{
    int i,j;

    //++++++++++++++++++++++++++++++++敵とアイテムの処理+++++++けっこうややこしい+++++
    for(i=0;i<Teki_NUM;i++)//画面外の敵は描画したくないための処理
    {
        //+++++++敵の処理+++++++++++++++++++++++++++++++++++++++++++++++++
        if(//画面内に敵がいる
            (teki[i].x<WindowWidth && x_char<WindowWidth/2) ||
            (abs(teki[i].x-x_char)<WindowWidth/2) ||
            (teki[i].x>Char_Size*(MapWidth-21) && (x_char>Char_Size*(MapWidth-11) || right_end_flag==1))
        )
        {
            if(teki[i].flag==0 && teki[i].out_flag==1)//前は画面外にいた
            {
                teki[i].flag=1;
                teki[i].out_flag=0;
            }
        }
        else//敵は画面外にいる
        {
            teki[i].flag=0;
            teki[i].out_flag=1;
        }
        //+++++++++++++++++++++敵の攻撃+++++++++++++++++
        if((abs(x_char-teki_attack[i].x)<Char_Size) && (abs(y_char-teki_attack[i].y)<Char_Size) && teki_attack[i].flag==1)
        {
            //+++++++++敵の攻撃を食らった+++++++++++++++++++++++
            Die();
        }

        if(teki[i].flag==1)//生存している敵のみ移動
        {
            if((abs(x_char-teki[i].x)<Char_Size) && (abs(y_char-teki[i].y)<Char_Size))
            {
                //++++++++++++++敵とぶつかった+++++++++++++++
                Die();
            }
            for(j=0;j<Attack_NUM;j++)
            {
                if((abs(attack[j].x-teki[i].x)<Char_Size) && (abs(attack[j].y-teki[i].y)<Char_Size) && attack[j].flag==1)
                {
                    if(j==1 || j==2)//FIRE,ICEの場合
                    {
                        attack[j].flag=0;//火か氷が消える
                        attack[j].t=0;
                    }

                    //+++++++++敵に攻撃した++++++++++++++++++
                    teki[i].flag=0;

                    if(item[i].flag==0 && (int)(5*(random()/(double)(RAND_MAX+1.0)))==0)//ランダムでアイテムを落とす
                    {
                        item[i].flag=1;//アイテムを表示するかどうかのフラグ
                        item[i].appear_flag=1;//アイテムが存在するかしないかのフラグ
                        item[i].x=teki[i].x;item[i].y=teki[i].y;
                    }
                }
            }

            Kabe_hantei(1,i);//tekiの判定
        }
        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

        //++++++++++++++++++++アイテムの処理++++++++++++++++++++++++++++++++++
        if(//画面内にアイテムがある
            (item[i].x<WindowWidth && x_char<WindowWidth/2) ||
            (abs(item[i].x-x_char)<WindowWidth/2) ||
            (item[i].x>Char_Size*(MapWidth-21) && x_char>Char_Size*(MapWidth-11))
        )
        {
            if(item[i].flag==0 && item[i].out_flag==1 && item[i].appear_flag==1)//前は画面外にあった
            {
                item[i].flag=1;
                item[i].out_flag=0;
            }
        }
        else//アイテムは画面外にある
        {
            item[i].flag=0;
            item[i].out_flag=1;
        }
        if(item[i].flag==1)//存在しているアイテムのみ判定
        {
            if((abs(x_char-item[i].x)<Char_Size) && (abs(y_char-item[i].y)<Char_Size))
            {
                //+++++++++++++++++++アイテムをゲットした++++++++++++++++++
                if(attack[item[i].kind+1].limit<3)//最大ストック3
                    attack[item[i].kind+1].limit++;//魔法の使用回数増加
                item[i].flag=0;//表示するかしないかのフラグ
                item[i].appear_flag=0;//アイテムが存在するかしないかのフラグ
            }
        }
        //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    }
}
void Boss_hantei(void)//+++++++++++++++++++++ボスの処理++++++++++++++++
{
    int i,j;

    if(boss.nodamage_t!=0)
        boss.nodamage_t--;

    for(i=0;i<Boss_Attack_NUM;i++)
    {
        for(j=0;j<3;j++)
        {
            //+++++++++++++++++++++ボスの攻撃+++++++++++++++++
            if((abs(x_char-boss_attack[i].x[j])<Char_Size) && (abs(y_char-boss_attack[i].y[j])<Char_Size) && boss_attack[i].flag==1)
            {
                //+++++++++++++++ボスの攻撃を食らった++++++++++++++++
                Die();
            }
        }
    }

    if(//+++++++++++++++++++++++ボスとの衝突判定+++++++サイズが違うためややこしい++++++
        (
            (x_char<=boss.x && (abs(x_char-boss.x)<Char_Size))//キャラのほうが左にいる場合
            ||
            (x_char>boss.x && (abs(x_char-boss.x)<Boss_Size))//キャラのほうが右にいる場合
        )
        &&
        (
            (y_char<=boss.y && (abs(y_char-boss.y)<Char_Size))//キャラのほうが上にいる場合
            ||
            (y_char>boss.y && (abs(y_char-boss.y)<Boss_Size))//キャラのほうが上にいる場合
        )
      )
    {
        //++++++ボスとぶつかった+++++++++++++++++++++++++++++++++++++
        Die();
    }
    for(j=0;j<Attack_NUM;j++)
    {
        if(//+++++++++++++++++++++++ボスへの攻撃判定+++++++サイズが違うためややこしい++++++
            (
                (attack[j].x<=boss.x && (abs(attack[j].x-boss.x)<Char_Size))//攻撃のほうが左にある場合
                ||
                (attack[j].x>boss.x && (abs(attack[j].x-boss.x)<Boss_Size))//攻撃のほうが右にある場合
            )
            &&
            (
                (attack[j].y<=boss.y && (abs(attack[j].y-boss.y)<Char_Size))//攻撃のほうが上にある場合
                ||
                (attack[j].y>boss.y && (abs(attack[j].y-boss.y)<Boss_Size))//攻撃のほうが下にある場合
            )
            &&
            attack[j].flag==1//攻撃を繰り出している
            &&
            boss.nodamage_t==0//無敵時間ではない
            &&
            die_t==0//主人公が死んでいる時ではない
          )
        {
            //++++++++++++++ボスに攻撃した++++++++++++++++++++++++++
            attack[j].flag=0;//攻撃が消える=連続で攻撃できない
            attack[j].t=0;
            if(j==0 || j==3)//剣か雷の場合は2ダメージ
                boss.life-=2;
            else
                boss.life--;
            if(boss.life<=0)//ボスを倒した
            {
                boss.flag=0;
                init();
                clear_flag=1;stage_start=0;
                Game_Clear();//ゲームクリア後のシーン開始
            }
            boss.nodamage_t=30;//ボスの無敵時間設定(連続で攻撃できない)
        }
    }

    Kabe_hantei(2,i);//bossの判定
}
void Kabe_hantei(int who,int n)    //+++++++++++移動先が壁かどうか判定+++++++++++++++++
{
    int cx,cy;
    int x,y;
    float t;
    int cursor,rand_tmp,teki_temp;//teki_tempは敵によって歩きを変えるため
    int step_temp;//敵とキャラの歩幅を変える
    int boss_n;//ボスの攻撃方法選択

    if(who==0)//キャラ移動の場合
    {
        x=x_char;
        y=y_char;
        cursor=cursor_flag;
        step_temp=Step;
        t=jump_t;

        cx=x/Char_Size;
        cy=y/Char_Size;

        if(jump_flag==1)//ジャンプ
        {
            cy=(y-1)/Char_Size;
            if(
                (map[cy][cx]=='C' || (x%Char_Size!=0 && map[cy][cx+1]=='C'))
                ||
                (JumpHeight-9.8*t<=0)
            )//壁に当たった場合かジャンプ力がなくなった場合
            {
                fall_flag=1;
                jump_flag=0;t=0;y_tempchar=y;
            }
            if(y>=0)
            {
                t+=0.24;
                y=y_tempchar - JumpHeight*t + 9.8*t*t/2;
                cy=y/Char_Size;
                if(map[cy][cx]=='C' || (x%Char_Size!=0 && map[cy][cx+1]=='C'))
                    y+=Char_Size-y%Char_Size;//ブロックに頭が埋まらないように
                if(JumpHeight-9.8*t<=0)//速度が逆向きになる
                {
                    fall_flag=1;
                    jump_flag=0;t=0;y_tempchar=y;
                }
            }
            else
            {//画面上に当たった場合
                fall_flag=1;
                jump_flag=0;t=0;y_tempchar=y;
            }
        }
        else if(!(map[cy+1][cx]=='C' || (x%Char_Size!=0 && map[cy+1][cx+1]=='C')))//落ちる
        {
            if(fall_flag==0)//横移動で落ち始めたとき
                y_tempchar=y;
            fall_flag=1;
            if(y+Char_Size+Step<WindowHeight)
            {
                t+=0.24;
                y=y_tempchar + 9.8*t*t/2;//落下処理
                cy=y/Char_Size;
                if(map[cy+1][cx]=='C' || (x%Char_Size!=0 && map[cy+1][cx+1]=='C'))//着地
                    y-=y%32;//ブロックに埋まらないように
            }
            else//落ちたとき
            {
                cursor=0;t=0;
            }
        }
        else//ジャンプ終了
        {
            fall_flag=0;t=0;
        }
    }
    else if(who==1)//敵の場合
    {
        x=teki[n].x;
        y=teki[n].y;
        if(n%4==0)//スライム
        {
            step_temp=Step-3;
            teki_temp=30;
        }
        else if(n%4==1)
        {
            step_temp=Step;
            teki_temp=20;
        }
        else
        {
            step_temp=Step-3;
            teki_temp=20;
        }
        cursor=teki[n].cursor;

        if(teki[n].t==teki_temp)
        {
            teki[n].walk_flag=teki[n].walk_flag^1;
            teki[n].t=0;
            if(n%4==2)//飛ぶ敵
                rand_tmp=5;
            else//歩く敵
                rand_tmp=3;
            if(cursor!=teki[n].direction || n%4==2)//歩く敵には移動制限を、飛ぶ敵にはなし
            {
                switch((int)(rand_tmp * (random()/(double)(RAND_MAX+1.0))))
                {
                    case 0 : cursor=0;teki[n].direction=(int)(2 * (random()/(double)(RAND_MAX+1.0)))+2;break;
                    case 1 : cursor=3;teki[n].direction=3;break;//2,3は横方向なので便宜上
                    case 2 : cursor=2;teki[n].direction=2;break;//3と
                    case 3 : cursor=1;break;//1が入れ替わっている
                    case 4 : cursor=4;break;
                };
            }

            if(n%4>=2)//飛ぶ敵と魔法使いは攻撃する
            {
                if(teki_attack[n].flag==0 && cursor==0)//敵が止まると攻撃
                {
                    if((teki[n].direction==2 && teki[n].x<x_char)//向いている方向にキャラがいたら
                     ||
                     (teki[n].direction==3 && teki[n].x>x_char))
                        teki_attack[n].flag=1;
                }
            }
        }
        teki[n].t++;
    }
    else if(who==2)//ボスの場合
    {
        x=boss.x;
        y=boss.y;

        step_temp=Step-2;//歩く速度
        teki_temp=40;//方向転換などの時間

        cursor=boss.cursor;

        if(boss.t==teki_temp)
        {
            boss.walk_flag=boss.walk_flag^1;
            boss.t=0;
            rand_tmp=2;//歩くボスには移動制限

            switch((int)(rand_tmp * (random()/(double)(RAND_MAX+1.0))))
            {
                case 0 : cursor=0;break;
                case 1 : cursor=boss.direction;break;//向いている方向へ進む
            };
            if(x_char<x)//キャラが左にいる場合
                boss.direction=3;
            else        //キャラが右にいる場合
                boss.direction=2;

            //++++++++++++ボスの攻撃++++++++++++++++++++++++++++++++
            boss_n=(int)(Boss_Attack_NUM * (random()/(double)(RAND_MAX+1.0)));
            if(boss_attack[0].flag==0 && boss_attack[1].flag==0 && boss_attack[2].flag==0 && boss_attack[3].flag==0 && cursor==0)//止まると攻撃
            {
                boss_attack[boss_n].flag=1;
            }
        }
        boss.t++;
    }

    cx=x/Char_Size;
    cy=y/Char_Size;
    switch(cursor)//移動先に壁があるかどうか
    {
        case 1://↑
            cy=(y-1)/Char_Size;
            if(map[cy][cx]=='C' || (x%Char_Size!=0 && map[cy][cx+1]=='C'))
            {
                cursor=0;
            }
            break;
        case 2://→
            if(who==2)//ボスの場合
            {
                if(map[cy][cx+2]=='C' || (y%Char_Size!=0 && map[cy+1][cx+2]=='C'))
                {
                    cursor=0;
                }
            }
            else
            {
                if(map[cy][cx+1]=='C' || (y%Char_Size!=0 && map[cy+1][cx+1]=='C'))
                {
                    cursor=0;
                }

                if(who==1 && n%4!=2)//歩く敵の場合、段差から落ちないように
                {
                    cx=(x+Char_Size)/Char_Size;
                    if(!(map[cy+1][cx]=='C' || (x%Char_Size!=0 && map[cy+1][cx+1]=='C')))
                    {
                        cursor=0;
                        teki[n].direction=3;
                    }
                }
            }
            break;
        case 3://←
            cx=(x-1)/Char_Size;
            if(map[cy][cx]=='C' || (y%Char_Size!=0 && map[cy+1][cx]=='C'))
            {
                cursor=0;
            }
            if(who==1 && n%4!=2)//歩く敵の場合、段差から落ちないように
            {
                cx=(x-Char_Size)/Char_Size;
                if(!(map[cy+1][cx]=='C' || (x%Char_Size!=0 && map[cy+1][cx+1]=='C')))
                {
                    cursor=0;
                    teki[n].direction=2;
                }
            }
            break;
        case 4://↓
            if(map[cy+1][cx]=='C' || (x%Char_Size!=0 && map[cy+1][cx+1]=='C'))
            {
                cursor=0;
            }
            break;
        case 0://キャラクター専用(壁を越える瞬間用)
        if(who==0)
        {
            if(cursor_key==2 && !(map[cy][cx+1]=='C' || (y%Char_Size!=0 && map[cy+1][cx+1]=='C')))
                cursor=2;
            else if(cursor_key==3)
            {
                cx=(x-1)/Char_Size;
                if(!(map[cy][cx]=='C' || (y%Char_Size!=0 && map[cy+1][cx]=='C')))
                    cursor=3;
            }
        }
        break;
    };

    switch(cursor)//画面外でなければ移動
    {
        case 1:if(y>=0){y-=step_temp;}else{cursor=0;}break;
        case 2:if(x+Char_Size+step_temp<=Char_Size*(MapWidth-1)){x+=step_temp;}else{cursor=0;}break;
        case 3:if(x-step_temp>=0){x-=step_temp;}else{cursor=0;}break;
        case 4:if(y+Char_Size+step_temp<WindowHeight){y+=step_temp;}else{cursor=0;}break;
    };

    if(who==0)
    {
        x_char=x;y_char=y;cursor_flag=cursor;jump_t=t;
    }
    else if(who==1)
    {
        teki[n].x=x;teki[n].y=y;teki[n].cursor=cursor;
    }
    else if(who==2)
    {
        boss.x=x;boss.y=y;boss.cursor=cursor;
    }
}
void Die(void)//死亡後の処理
{
    if(die_t==0)//死亡アニメ開始
    {
        die_t=120;
        cursor_flag=0;cursor_key=0;
    }

    die_t--;
    if(die_t==0)//死亡終わり
    {
        Stage_Change();
    }
}
void Stage_Change(void)//ステージを変える
{
    int i;
    char file_name[20];

    init();//初期化

    stage_start=0;//ステージが開始されない
    pause_flag=1;

    if(stage==0)//タイトル画面
    {
        //++++++++++++タイトル描画++++++++++++++++++++++++
        glClear(GL_COLOR_BUFFER_BIT);
        glBindTexture(GL_TEXTURE_2D,scene_texture[0]);
        PutSprite(0,0,WindowWidth,WindowHeight);
        glFlush();
        glutSwapBuffers();
    }
    else if(stage==1)
    {
        //++++++++++++NOW LOADING++++++++++++++++++++++++
        glClear(GL_COLOR_BUFFER_BIT);
        glBindTexture(GL_TEXTURE_2D,scene_texture[1]);
        PutSprite(0,0,WindowWidth,WindowHeight);
        glFlush();
        glutSwapBuffers();

        //+++++++++++マップ画像読み込み++++++++++++++++++++++++++++
        for(i=0;i<Tex_NUM;i++)
        {
            sprintf(file_name,"image/map_%d.png",i+1);
            map_texture[i] = pngBind(file_name, PNG_NOMIPMAP, PNG_ALPHA, &map_info[i], GL_CLAMP, GL_NEAREST, GL_NEAREST);
        }

        strcpy(map[ 0],"CAAAAAAAAAAAAAAAAAAAACAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAABAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAACA");
        strcpy(map[ 1],"CAAAAAAABAAAAAAAAAAAACAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAABACA");
        strcpy(map[ 2],"CAAAAAAAAAAAAAAAAAAAACAAAAAAAAAAAAAAAAAAAAAAAAABAAAAAAAAACAAAAAAAAABAAAAAAACCCCCCCAAAAAAAAAAAAAAAACA");
        strcpy(map[ 3],"CAAAAAAAAAAAAAAAAAACACAAAAAAAAAAAAABAAAAAAAAAAAAAAAAAAAAACAAACCAAAAAAAAAAAAAAAAAAAAACCCAAACCCAAACCCA");
        strcpy(map[ 4],"CBAACCCCCAAAAAAAACCCACCCAAAAACAAAAAAAAAAAAAAAAAAAACCAAAAACCAAAAAAAAAACCCAAAAAAABAAAAAAAAAAAAAAAAAACA");
        strcpy(map[ 5],"CAAAAAAABAAAAAAAACCCAAAAAAAAACAAAAAAAAAAAAABAAAAAAAAAAAAACAAAAABAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAACA");
        strcpy(map[ 6],"CACAAAAAAAAACCCCCCCCAABAAAACCCAAACAAAAAAAAAAAAACCAAAAAACACCCAAAAAAAAAAAAAAAAACCCAAAAAAAAAAABAAAAAACA");
        strcpy(map[ 7],"CACCAAAAAAAAAAAACCCCAAAAAAAAACAAABACABAAAAAAAAAACCAAAAACACAAAACCAAAACCCAAAAAAAAAAAAAAAAAAAAAAAAAAACA");
        strcpy(map[ 8],"CAACCAAAAAAAAAAACCCCAACCCAAAACAAAAAAACAAAAAAAAAAACCAAAACACAAAAAAAAAAAAAAAABAAAAAAABAAAAAAAAAAAAAAACA");
        strcpy(map[ 9],"AAAACCCAAAAAABAACCCCAAAAAAAAACAAAAAAABACAAAAAAAAAAAAACCCACAACAAABAAAAAACCCAAAAAAAAAAAAAAAAAAABAAAACA");
        strcpy(map[10],"AAAAAACCCAAAAAAACCCCAABAAAACCCAAAACAAAAAAAAAAABAAAAAAAACACAAAAAAAAAAAAAAAAAAACCCAAAAAAAAAAAAAAAAAACA");
        strcpy(map[11],"AAAAAAAACCCAAAAACCCCAAAACCAAACAAAAAAAAAACAAAAAACCCCCCCCCACCCCCAAAAAAAAAAAAAAAAAAAAAAAAAAABAAAAAAAAAA");
        strcpy(map[12],"AAAAAAAAAAAAAAAACCCCAAAAAAAAACAAAAAAAAAAAAAAAAACAAAAAAAAAAAAAAAAAAABAACAAAAAABAAAAAAAAAAAAAAAAAAAAAA");
        strcpy(map[13],"CCCCCCCCCCCCCCCCCCCCCCCAAAAAACCCCCCCCCCCCCAACCCCCCCCCCCCCCCCCCCAACAAAACAABACAAAACABAACCCCCCCCCCCCCCC");
        strcpy(map[14],"CCCCCCCCCCCCCCCCCCCCAAAAABAAACCCCCCCCCCCCCAACCCCCCCCCCCCCCCCCCCAACAAAACAAAACAAAACAAAACCCCCCCCCCCCCCC");

        //++++++++++++敵の初期設定+++++++++++++++++++++++
        teki[0].x=Char_Size*6;teki[0].y=Char_Size*8;
        teki[1].x=Char_Size*5;teki[1].y=Char_Size*3;
        teki[2].x=Char_Size*27;teki[2].y=Char_Size*3;
        teki[3].x=Char_Size*22;teki[3].y=Char_Size*3;
        teki[4].x=Char_Size*12;teki[4].y=Char_Size*12;
        teki[5].x=Char_Size*28;teki[5].y=Char_Size*9;
        teki[6].x=Char_Size*35;teki[6].y=Char_Size*3;
        teki[7].x=Char_Size*40;teki[7].y=Char_Size*12;
        teki[8].x=Char_Size*37;teki[8].y=Char_Size*12;
        teki[9].x=Char_Size*24;teki[9].y=Char_Size*7;
        teki[10].x=Char_Size*45;teki[10].y=Char_Size*2;
        teki[11].x=Char_Size*58;teki[11].y=Char_Size*10;
        teki[12].x=Char_Size*62;teki[12].y=Char_Size*2;
        teki[13].x=Char_Size*59;teki[13].y=Char_Size*12;
        teki[14].x=Char_Size*50;teki[14].y=Char_Size*7;
        teki[15].x=Char_Size*72;teki[15].y=Char_Size*8;
        teki[16].x=Char_Size*78;teki[16].y=Char_Size*9;
        teki[17].x=Char_Size*69;teki[17].y=Char_Size*3;
        teki[18].x=Char_Size*78;teki[18].y=Char_Size*5;
        teki[19].x=Char_Size*79;teki[19].y=Char_Size*1;
        teki[20].x=Char_Size*90;teki[20].y=Char_Size*12;
        teki[21].x=Char_Size*94;teki[21].y=Char_Size*12;
        teki[22].x=Char_Size*95;teki[22].y=Char_Size*2;
        teki[23].x=Char_Size*96;teki[23].y=Char_Size*12;
        teki[24].x=Char_Size*93;teki[24].y=Char_Size*12;

        sleep(1);//ローディング画面のウェイト

        //++++++++++++STAGE開始の背景++++++++++++++++++++++++
        glClear(GL_COLOR_BUFFER_BIT);
        glBindTexture(GL_TEXTURE_2D,scene_texture[2]);
        PutSprite(0,0,WindowWidth,WindowHeight);
        glFlush();
        glutSwapBuffers();
    }
    else if(stage==2)//stage2
    {
        //++++++++++++NOW LOADING++++++++++++++++++++++++
        glClear(GL_COLOR_BUFFER_BIT);
        glBindTexture(GL_TEXTURE_2D,scene_texture[1]);
        PutSprite(0,0,WindowWidth,WindowHeight);
        glFlush();
        glutSwapBuffers();

        //+++++++++++マップ画像読み込み++++++++++++++++++++++++++++
        for(i=0;i<Tex_NUM;i++)
        {
            sprintf(file_name,"image/map2_%d.png",i+1);
            map_texture[i] = pngBind(file_name, PNG_NOMIPMAP, PNG_ALPHA, &map_info[i], GL_CLAMP, GL_NEAREST, GL_NEAREST);
        }

        strcpy(map[ 0],"CAAAAAAAAAAAAAAAAAAAACAAAAAAAAAAAAAAAAAAAAAACAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAACAAAAAAAAAAAAAAAAAAAAA");
        strcpy(map[ 1],"CAAAABAAAAAAAAAAAAAAACAAAAAAAAAAAAAAAAAAAAAACAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAACAAAAAAAAABAAAAAAAAAAA");
        strcpy(map[ 2],"CAAAAAAAAAAAAAAAAAAAAAAAAAAAABAAAAAAAAAAAAAACAAAAAAACCCCCCCCCCCCCCCCAAAAAAAAAACAAAAAAAAAAAAAAAAAAAAA");
        strcpy(map[ 3],"CAAAAAAAAAAAAAAACAAAAAAAAAAAAAAAAAAAAACAAAAACAAAAAAAAAAAAAAAAAAAAAACAAAAAAAAAACAAAAAAAAAAAAAAAAAAAAA");
        strcpy(map[ 4],"CAAAAAAAACAAAAAAAAAAACAACCCCCCCCCAACCCCCCCAACAAAAAAAAAAAAAAAAAAAAAACAAAAAAABAACAAAAAAAAAAAABAAAAAAAA");
        strcpy(map[ 5],"CAAAAAAAAAAAAAAAAAAAACAAAAAAAAAACCCCAAAAAAAACCCCCCCCCCCCCCCAAAAAAAACAAAAAAAAAACAAAAAAAAAAAAAAAAAAAAA");
        strcpy(map[ 6],"CAAAACAAAABACAAABACAACAAAAAAAAAAACAAAAAAAAAACAAAAAAAAAAAAAAAACCAAAACAAAAAAAAAACAAAAAAAAAAAAAAAAAAAAA");
        strcpy(map[ 7],"CAAAAAAAAAAAAAAAAAAAACAACCCCCCCAACAAACCCCCCCCAAAAAAAAAAAAAAAAAAAAAACAAAAAAAAAACAAAAAACCAAAACCAAAAAAA");
        strcpy(map[ 8],"CAAAAAACAAAAAAACAAAAACAAAAAAAAAAACCAAAAABAAAAAAAAAAAAAAAAAAAAAAAACACAAAAAAAAAACAAAAAAAAAAAAAAAAAAAAA");
        strcpy(map[ 9],"CAAAAAAAABAACAAAAAAAACAAAAAAAAAAAAAAAAAAAAAAAAAAABAAAAAAAAAAAAAAAABCAAAAAAAAAACAAAAAABAAAAAAAAAAAAAA");
        strcpy(map[10],"CAAAAAAAAAAAAAAAAAAAACCCAAAAAAACCAACCAAAAAAAAAAAAAAAAAAAAAAAAAAACAACAAAAAACCAACAAACCAAAACCAAAACCAAAA");
        strcpy(map[11],"AAAAAACAAACAAAAACAAAACAAAACAACAAAAAAAAAAAAAAAAAAAAAAAAAAAAACAAAAAAACAAAAAAABAAAAAAAAAAAAAAAAAAAAAAAA");
        strcpy(map[12],"AAAAAAAAAAAAAAAAAAAAACAAAACCCCAAACCAAAACCAAAAAAAAAAAAACBAAAAAAAAAAACAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA");
        strcpy(map[13],"CCCCCCCCCCCCCCCCCCCCCCAAAAAAAAAAAAAAAAAAAAACAAAAACAAAAAAAAAAAAAAAAACCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC");
        strcpy(map[14],"CCCCCCCCCCCCCCCCCCCCCCAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAACCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC");

        //++++++++++++敵の初期設定+++++++++++++++++++++++
        teki[0].x=Char_Size*6;teki[0].y=Char_Size*12;
        teki[1].x=Char_Size*14;teki[1].y=Char_Size*12;
        teki[2].x=Char_Size*23;teki[2].y=Char_Size*12;
        teki[3].x=Char_Size*16;teki[3].y=Char_Size*2;
        teki[4].x=Char_Size*12;teki[4].y=Char_Size*12;
        teki[5].x=Char_Size*28;teki[5].y=Char_Size*3;
        teki[6].x=Char_Size*35;teki[6].y=Char_Size*9;
        teki[7].x=Char_Size*40;teki[7].y=Char_Size*3;
        teki[8].x=Char_Size*37;teki[8].y=Char_Size*3;
        teki[9].x=Char_Size*24;teki[9].y=Char_Size*6;
        teki[10].x=Char_Size*45;teki[10].y=Char_Size*2;
        teki[11].x=Char_Size*58;teki[11].y=Char_Size*1;
        teki[12].x=Char_Size*62;teki[12].y=Char_Size*1;
        teki[13].x=Char_Size*59;teki[13].y=Char_Size*1;
        teki[14].x=Char_Size*50;teki[14].y=Char_Size*7;
        teki[15].x=Char_Size*72;teki[15].y=Char_Size*12;
        teki[16].x=Char_Size*78;teki[16].y=Char_Size*12;
        teki[17].x=Char_Size*69;teki[17].y=Char_Size*12;
        teki[18].x=Char_Size*74;teki[18].y=Char_Size*5;
        teki[19].x=Char_Size*79;teki[19].y=Char_Size*12;
        //++++++以下の敵はゲームには参加しない+++++++++++++++++
        teki[20].x=Char_Size*150;teki[20].y=Char_Size*12;
        teki[21].x=Char_Size*150;teki[21].y=Char_Size*12;
        teki[22].x=Char_Size*150;teki[22].y=Char_Size*12;
        teki[23].x=Char_Size*150;teki[23].y=Char_Size*12;
        teki[24].x=Char_Size*150;teki[24].y=Char_Size*12;
        boss.x=Char_Size*96;boss.y=Char_Size*11;boss.life=20;

        sleep(1);//ローディング画面のウェイト

        //++++++++++++STAGE開始の背景++++++++++++++++++++++++
        glClear(GL_COLOR_BUFFER_BIT);
        glBindTexture(GL_TEXTURE_2D,scene_texture[3]);
        PutSprite(0,0,WindowWidth,WindowHeight);
        glFlush();
        glutSwapBuffers();
    }
}
void Game_Clear(void)
{
    if((z_key==1 && z_up==0) || clear_scene==4)//押しっぱなしでも進まない。zを押したときとクリア直後
    {
        z_key=0;

        if(clear_scene==Scene_NUM)//Finまで表示した
        {
            stage=0;
            Stage_Change();
            return;
        }

        //++++++++++++タイトル描画++++++++++++++++++++++++
        glClear(GL_COLOR_BUFFER_BIT);
        glBindTexture(GL_TEXTURE_2D,scene_texture[clear_scene]);
        PutSprite(0,0,WindowWidth,WindowHeight);
        glFlush();
        glutSwapBuffers();

        clear_scene++;
        sleep(1);
    }
}

void Keyboard(char  key,int x,int y)//キー入力
{
    if(die_t!=0)//死亡中
        return;
    if(pause_flag==0)
    {
        switch(key)
        {
            case 'x':if(jump_flag==0 && jump_key==0 && fall_flag==0)
                    {
                        jump_key=1;
                        jump_flag=1;
                        y_tempchar=y_char;
                    }
                    break;
            case 'z':if(attack[0].flag==0 && z_key==0){attack[0].flag=1;z_key=1;z_up=0;attack[0].x=0;attack[0].y=0;}break;
            case 'a':if(attack[1].flag==0 && a_key==0 && attack[1].limit!=0){attack[1].flag=1;a_key=1;attack[1].limit--;attack[1].x=0;attack[1].y=0;}break;
            case 's':if(attack[2].flag==0 && s_key==0 && attack[2].limit!=0){attack[2].flag=1;s_key=1;attack[2].limit--;attack[2].x=0;attack[2].y=0;}break;
            case 'd':if(attack[3].flag==0 && d_key==0 && attack[3].limit!=0){attack[3].flag=1;d_key=1;attack[3].limit--;attack[3].x=0;attack[3].y=0;}break;

            default:break;
        }
    }
    else if(key=='z' && z_up==1)//シーン中にZを押すと1場面進む
    {
        z_key=1;z_up=0;
        if(stage_start==0 && clear_flag==0)
        {
            stage_start=1;
            pause_flag=0;
            if(stage==0)//ゲーム開始
            {
                stage=1;
                Stage_Change();
            }
        }
    }
    if(key==32 && stage_start!=0 && stage!=0)//SPACEはASCIIで32
    {//ゲーム中にポーズ
        if(pause_key==0)
        {
            pause_flag^=1;pause_key=1;
        }
    }
    else if(key==27)
    {
        exit(1);//ゲーム終了
    }
}
void KeyboardUp(char key,int x,int y)//キーUP処理
{
    if(key==32)//ポーズ解除
        pause_key=0;
    if(key=='z' && stage_start==0)//シーン中
    {
        z_key=0;z_up=1;
    }
    if(pause_flag==1)
        return;
    switch(key)
    {
        case 'x':jump_key=0;break;
        case 'z':z_key=0;break;
        case 'a':a_key=0;break;
        case 's':s_key=0;break;
        case 'd':d_key=0;break;
        case 32:pause_key=0;break;

        default:break;
    }
}
void SpecialKey(int key,int x,int y)//キー処理
{
    if(die_t!=0)//死亡中
        return;
    switch(key)//人間は上と下には移動できないが、敵はできるため
    {//cursor_flagは2と3
        case GLUT_KEY_RIGHT:cursor_flag=2;direction=2;cursor_key=2;break;
        case GLUT_KEY_LEFT:cursor_flag=3;direction=3;cursor_key=3;break;

        default:break;
    }
}
void SpecialUpKey(int key,int x,int y)//キーUP処理
{
    switch(key)
    {
        case GLUT_KEY_RIGHT:if(cursor_flag==2 || cursor_flag==0){cursor_flag=0;cursor_key=0;}break;
        case GLUT_KEY_LEFT:if(cursor_flag==3 || cursor_flag==0){cursor_flag=0;cursor_key=0;}break;

        default:break;
    }
}
