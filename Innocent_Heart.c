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
void PutMap(char map,int x,int y,int n);//�}�b�v�̕\��
void PutChar(void);//�L�����̕\��
void PutTeki(int n,int what);//�G�L����,�A�C�e���̕\��
void PutMagic(int what);//���@�̎c��g�p���\��
void PutLife(void);//�{�X�̎c�胉�C�t�\��
void PutSprite(int x,int y,int width,int height);//�摜�\��t��
void PutSprite_RE(int x,int y,int width,int height);//�摜���]�\��t��
void Attack(int what);//�U������
void Attack_teki(int n);//�G�̍U������
void Attack_boss(int n);//�{�X�̍U������
void Anime(int what,int teki_n,int boss_n);//�U���Ȃǂ̃A�j��
void Keyboard(char key,int x,int y);//�L�[����
void KeyboardUp(char key,int x,int y);//�L�[����
void SpecialKey(int key,int x,int y);//�L�[����
void SpecialUpKey(int key,int x,int y);//�L�[UP����
void Teki_hantei(void);//�G�Ƃ̏Փ˂�G�̍U���A�A�C�e���̔���Ȃǂ���������
void Boss_hantei(void);//�{�X�Ƃ̏Փ˂�U������������
void Kabe_hantei(int who,int n);//�ǔ���
void Die(void);//���S��̏���
void Stage_Change(void);//�X�e�[�W��ς���
void Game_Clear(void);//�Q�[���N���A��̏���

#define WindowWidth 640//�E�B���h�E�̑傫��
#define WindowHeight 480
#define MapWidth 100
#define MapHeight 15
#define Tex_NUM 3//�}�b�v�C���[�W��
#define Scene_NUM 14//�V�[���̃C���[�W��
#define NUM_IMAGE 4//�����摜�̐�
#define Char_NUM 3
#define Teki_NUM 25
#define Attack_NUM 4//�U���̎��
#define Boss_Attack_NUM 4//�{�X�̍U���̎��
#define Char_Size 32//�C���[�W�T�C�Y
#define Boss_Size 64//�{�X�T�C�Y
#define Life_Size 16//�{�X�̃��C�t�T�C�Y
#define Step 4
#define JumpHeight 44//�W�����v��(�����x)

struct attack_anime//�U���̑���
{
    int x,y,t;//t�͎���
    int flag;//�U�������ǂ���
    int direction;
    int limit;

    pngInfo info[3];
    GLuint texture[3];
};

struct teki_status//�G�̏��
{
    int x,y,cursor,t,walk_flag;
    int flag,out_flag;//�����Ă��邩�ǂ���,��ʂ̊O�ɂ������ǂ���
    int direction;

    pngInfo info[2];
    GLuint texture[2];
};

struct teki_attack_anime//�G�̍U��
{
    int x,y,t;
    float x_move,y_move;//move�͈ړ���
    float x_sum,y_sum;
    int flag;
    int direction;

    pngInfo info[3];
    GLuint texture[3];
};

struct item_status//�A�C�e���̏��
{
    int x,y,kind;//kind�̓A�C�e���̎��
    int flag,out_flag,appear_flag;
    //flag�͕\�����邩���Ȃ���
    //out_flag�͉�ʊO���ǂ���
    //appear_flag�͑��݂��Ă��邩�ǂ���
    pngInfo info[1];
    GLuint texture[1];
};

struct boss_status//�{�X�̏��
{
    int x,y,cursor,t,walk_flag;
    int flag;//�����Ă��邩�ǂ���
    int direction;
    int life,nodamage_t;

    pngInfo info[2];
    GLuint texture[2];
};

struct boss_attack_anime//�{�X�̍U��
{
    int x[3],y[3],t;
    int x_move[3],y_move[3];
    int flag;
    int direction;

    pngInfo info[3];
    GLuint texture[3];
};

//+++++++++++++++++++++++++++++++++++++�e�N�X�`��+++++++++++++++++++++++++++++++++
pngInfo map_info[Tex_NUM+1],scene_info[Scene_NUM],Char_info[Char_NUM],num_info[NUM_IMAGE],item_info[Attack_NUM-1],life_info;
GLuint  map_texture[Tex_NUM],scene_texture[Scene_NUM],Char_texture[Char_NUM],num_texture[NUM_IMAGE],item_texture[Attack_NUM-1],life_texture;

int walk_flag=0,walk_count=0,right_end_flag=0;
int x_char,y_char,y_tempchar=0;
int cursor_flag=0,cursor_key=0,direction=2;//cursor=0:�Ȃ�,1:��,2:�E,3:��,4:�����J�[�\�����͂���
int jump_flag=0,jump_key;
int stage=0,stage_start=0;//�V�[���p�t���O
float jump_t;
int z_key,a_key,s_key,d_key,fall_flag=0,pause_flag=1,pause_key=0,die_t;
int clear_flag,clear_scene,z_up,change_flag;

//++++++++++++�U���̍\���̐ݒ�+++++++++++++++
static struct attack_anime attack[Attack_NUM];//�U���̎�ސ�
static struct teki_status teki[Teki_NUM];//�G�̐�
static struct teki_attack_anime teki_attack[Teki_NUM];//�G�̍U��
static struct item_status item[Teki_NUM];//�A�C�e���̐�=�G�̐�
static struct boss_status boss;//�{�X���
static struct boss_attack_anime boss_attack[Boss_Attack_NUM];//�{�X�̍U��

char map[MapHeight][MapWidth];

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

    width=glutGet(GLUT_SCREEN_WIDTH);//�f�B�X�v���C�̕�
    height=glutGet(GLUT_SCREEN_HEIGHT);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

    //+++++++++++���[�h��ʂ̉摜�ǂݍ���++++++++++++++++++++++++++++
    for(i=0;i<Scene_NUM;i++)
    {
        sprintf(file_name,"image/scene_%d.png",i+1);
        scene_texture[i] = pngBind(file_name, PNG_NOMIPMAP, PNG_ALPHA, &scene_info[i], GL_CLAMP, GL_NEAREST, GL_NEAREST);
    }

    //+++++++++++�L�����摜�ǂݍ���+++++++++++++++++++++++++++++
    for(i=0;i<Char_NUM;i++)
    {
        sprintf(file_name,"image/char_%d.png",i+1);
        Char_texture[i] = pngBind(file_name, PNG_NOMIPMAP, PNG_ALPHA, &Char_info[i], GL_CLAMP, GL_NEAREST, GL_NEAREST);
    }

    //++++++++++++�G�̉摜�ǂݍ���+++++++++++++++
    for(j=0;j<Teki_NUM;j++)
    {
        for(i=0;i<2;i++)
        {
            sprintf(file_name,"image/teki%d_%d.png",j%4+1,i+1);
            teki[j].texture[i] = pngBind(file_name, PNG_NOMIPMAP, PNG_ALPHA, &teki[j].info[i], GL_CLAMP, GL_NEAREST, GL_NEAREST);
        }
    }

    //++++++++++++�G�̍U���̉摜�ǂݍ���+++++++++++++++
    for(j=0;j<Teki_NUM;j++)
    {
        for(i=0;i<3;i++)
        {
            if(j%4==2)//�f�[�����̖��@
                sprintf(file_name,"image/teki_attack_%d.png",i+1);
            else//�E�B�U�[�h�̖��@
                sprintf(file_name,"image/teki_attack_wind_%d.png",i+1);
            teki_attack[j].texture[i] = pngBind(file_name, PNG_NOMIPMAP, PNG_ALPHA, &teki_attack[j].info[i], GL_CLAMP, GL_NEAREST, GL_NEAREST);
        }
    }
    //++++++++++++�{�X�̍U���̉摜�ǂݍ���+++++++++++++++
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

    //++++++++++++�A�C�e���̉摜�ǂݍ���+++++++++++++++
    for(j=0;j<Teki_NUM;j++)
    {
        for(i=0;i<1;i++)
        {
            item[j].kind=3*(random()/(double)(RAND_MAX+1.0));//�A�C�e���̎�ނ������_���őI��
            sprintf(file_name,"image/item_%d.png",item[j].kind+1);
            item[j].texture[i] = pngBind(file_name, PNG_NOMIPMAP, PNG_ALPHA, &item[j].info[i], GL_CLAMP, GL_NEAREST, GL_NEAREST);
        }
    }

    //++++++++++++�c�薂�@�g�p�񐔂̉摜�ǂݍ���+++++++++++++++
    for(i=0;i<Attack_NUM-1;i++)//3���
    {
        sprintf(file_name,"image/item_%d.png",i+1);
        item_texture[i] = pngBind(file_name, PNG_NOMIPMAP, PNG_ALPHA, &item_info[i], GL_CLAMP, GL_NEAREST, GL_NEAREST);
    }

    //++++++++++++�{�X�̎c�胉�C�t�̉摜�ǂݍ���+++++++++++++++
    sprintf(file_name,"image/life.png");
    life_texture = pngBind(file_name, PNG_NOMIPMAP, PNG_ALPHA, &life_info, GL_CLAMP, GL_NEAREST, GL_NEAREST);

    //++++++++++++�����摜�ǂݍ���+++++++++++++++
    for(i=0;i<NUM_IMAGE;i++)
    {
        sprintf(file_name,"image/num_%d.png",i);
        num_texture[i] = pngBind(file_name, PNG_NOMIPMAP, PNG_ALPHA, &num_info[i], GL_CLAMP, GL_NEAREST, GL_NEAREST);
    }

    //++++++++++++�U���̉摜�ǂݍ���+++++++++++++++
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
    //++++++++++++�{�X�̉摜�ǂݍ���+++++++++++++++
    for(i=0;i<2;i++)
    {
        sprintf(file_name,"image/boss_%d.png",i+1);
        boss.texture[i] = pngBind(file_name, PNG_NOMIPMAP, PNG_ALPHA, &boss.info[i], GL_CLAMP, GL_NEAREST, GL_NEAREST);
    }
    
    init();//������
    
    glEnable(GL_BLEND);//�e�N�X�`���̃A���t�@�`�����l���L��
    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
    glTexEnvf(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);

    glEnable(GL_CULL_FACE);
    
    glutDisplayFunc(Display);//Display��`
    glutReshapeFunc(Reshape);//Reshape��`
    glutTimerFunc(500,Timer,0);//0.5�b���Ăяo��

    glutKeyboardFunc(Keyboard);
    glutKeyboardUpFunc(KeyboardUp);
    glutSpecialFunc(SpecialKey);
    glutSpecialUpFunc(SpecialUpKey);
    
    glutMainLoop();

    return(0);
}

void init(void)//������ �R���e�j���[���ȂǂŌĂ�
{
    int i;
    
    cursor_flag=0;cursor_key=0;direction=2;
    jump_flag=0;fall_flag=0;jump_t=0;
    if(boss.flag==1)//������̃L�����N�^�[�̈ʒu(�{�X��)
    {
        x_char=Char_Size*75;y_char=Char_Size*12;
    }
    else
    {
        x_char=Char_Size*2;y_char=Char_Size*12;
        boss.flag=0;
    }
    right_end_flag=0;//�E�[�̃J�����Œ������
    pause_flag=1;
    die_t=0;//���S����0=����ł��Ȃ�
    
    for(i=0;i<Attack_NUM;i++)
    {
        attack[i].flag=0;
        attack[i].t=0;
    }
    for(i=0;i<Teki_NUM;i++)
    {
        teki[i].flag=1;
        teki[i].out_flag=0;
        teki[i].direction=2;//�����ݒ�ł͓G�͉E�������Ă���
        teki[i].cursor=0;
        item[i].flag=0;//�A�C�e��������
        item[i].out_flag=0;
        item[i].appear_flag=0;
        teki_attack[i].flag=0;//�G�̍U��������
        teki_attack[i].t=0;
    }
    boss.direction=3;boss.cursor=0;
    boss.nodamage_t=0;
    for(i=0;i<Boss_Attack_NUM;i++)
    {
        boss_attack[i].flag=0;//�{�X�̍U��������
        boss_attack[i].t=0;
    }
    
    for(i=0;i<Attack_NUM;i++)
        attack[i].limit=3;//���@�g�p�񐔏�����
    clear_flag=0;clear_scene=4;z_up=1;//�Q�[���N���A�p
    change_flag=0;
}

//
//�E�B���h�E�̕\�����e���X�V����֐�
//
void Display(void)
{
    int i,j,n=0;
    
    if(stage==0 || stage_start==0)//�^�C�g����ʂ��X�e�[�W�J�n�����N���A��
    {
        return;
    }
    
    glClear(GL_COLOR_BUFFER_BIT);

    if(x_char > WindowWidth/2)//�^�񒆂��E�ɍs�����Ƃ�
    {
        n=(x_char-WindowWidth/2)/Char_Size;
        if(x_char >= Char_Size*(MapWidth-11) || right_end_flag==1)//�E�[
        {
            n=MapWidth-21;//��ʌŒ�
            if(x_char==Char_Size*(MapWidth-2) && stage==1)//���̃X�e�[�W��
            {
                stage=2;
                Stage_Change();
                return;
            }
        }
    }

    for(i=0;i<15;i++)//�}�b�v�\��
    {
        for(j=0;j<=20;j++)
        {
            PutMap(map[i][j+n],j,i*map_info->Height,n);
        }
    }
    
    //+++++++++++�e�L�����N�^�[��U���̏���+++++++++++++++++++
    PutChar();
    if(boss.flag==1)//�{�X��
    {
        for(i=0;i<Boss_Attack_NUM;i++)
        {
            if(boss_attack[i].flag==1)//�{�X�̍U��
                Attack_boss(i);
        }
        PutTeki(0,2);//�{�X�̕\��
        PutLife();//�{�X�̃��C�t�\��
    }
    else
    {
        for(i=0;i<Teki_NUM;i++)
        {
            if(teki_attack[i].flag==1)//�G�̍U��
                Attack_teki(i);
            if(item[i].flag==1)
                PutTeki(i,1);//�A�C�e���̕\��
            if(teki[i].flag==1)
                PutTeki(i,0);//�G�̕\��
        }
    }
    for(i=0;i<Attack_NUM;i++)
    {
        if(i!=0)//���@�̎c��g�p�񐔂�\��
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

    if(pause_flag==0)//�Q�[�����s��
    {
        Kabe_hantei(0,0);//char�̕ǔ���

        if(right_end_flag==1)//�{�X��
            Boss_hantei();
        else if(boss.flag==0)
            Teki_hantei();//�G�Ƃ̏Փ˂�U���A�A�C�e���̔���̏���

        if(y_char+Char_Size>=WindowHeight-Char_Size)//���܂ŗ������Ƃ�
        {
            //+++++++++++++++++++������++++++++++++++++++++++++++
            Stage_Change();
        }

        Display();
        walk_count++;
        if(walk_count==20)
        {
            walk_count=0;
            walk_flag=walk_flag^1;//���s�G�؂�ւ�
        }
        
        if(die_t!=0)//���񂾂Ƃ�
            Die();
    }
    else if(stage==0 && change_flag==0)//�^�C�g�����
    {
        Stage_Change();
        change_flag=1;
    }
    else if(clear_flag==1)//�N���A��̏���
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
void PutMap(char map,int x,int y,int n)//�}�b�v�\��
{
    int tex;

    switch(map)
    {
        case 'A' : tex=0; break;
        case 'B' : tex=1; break;
        case 'C' : tex=2; break;
        default : printf("ERROR\n");exit(1);
    };

    if(x_char >= Char_Size*(MapWidth-11) || right_end_flag==1)//��ʉE�[
        x*=map_info->Width;//�E�[�܂ōs���ƈ����Ԃ��Ȃ��Ȃ�
    else if(x_char>=WindowWidth/2)//�X�N���[��
        x=x*Char_Size+WindowWidth/2-x_char+n*Char_Size;
    else
        x*=map_info->Width;

    glBindTexture(GL_TEXTURE_2D, map_texture[tex]);
        
    PutSprite(x,y,Char_Size,Char_Size);
}
void PutChar(void)//�L�����\��
{
    int x;
    int tex;
    if(x_char >= Char_Size*(MapWidth-11) || right_end_flag==1)//��ʉE�[
    {
        x=x_char-Char_Size*(MapWidth-21);
        if(stage==2 && right_end_flag==0)//stage2�̂݃{�X��ŃJ�����Œ�̂���
        {
            right_end_flag=1;//�E�[�܂ōs���ƈ����Ԃ��Ȃ��Ȃ�
            boss.flag=1;//�{�X�o��
            map[11][78]='C';//�{�X��ŕǐ���
            map[12][78]='C';
            map[11][99]='C';
        }
    }
    else if(x_char>=WindowWidth/2)//�X�N���[��
        x=WindowWidth/2;
    else
        x=x_char;

    tex=walk_flag;//�\���摜(�����Ă���)
    if(die_t!=0)//����ł��邩�ǂ���
        tex=2;//���񂾉摜

    glBindTexture(GL_TEXTURE_2D, Char_texture[tex]);

    if(direction==2)
        PutSprite_RE(x,y_char,Char_Size,Char_Size);//�摜���]
    else
        PutSprite(x,y_char,Char_Size,Char_Size);//�摜�\��t��
}
void PutTeki(int n,int what)//�G�\��,�A�C�e���\��,�{�X�\��
{
    int x,y,size;
    int direction;
    GLuint texture;

    switch(what)
    {
        case 0:x=teki[n].x;y=teki[n].y;texture=teki[n].texture[teki[n].walk_flag];size=Char_Size;direction=teki[n].direction;break;
        case 1:x=item[n].x;y=item[n].y;texture=item[n].texture[0];size=Char_Size;break;
        case 2:x=boss.x;y=boss.y;texture=boss.texture[boss.walk_flag];size=Boss_Size;direction=boss.direction;
                if(boss.nodamage_t%10>7)//�{�X�ɍU��������̖��G���Ԃł̓{�X���_�ł���
                    return;
                break;
    };

    if(x_char >= Char_Size*(MapWidth-11) || right_end_flag==1)
        x=x-Char_Size*(MapWidth-21);
    else if(x_char>=WindowWidth/2)
        x=x-x_char+WindowWidth/2;

    glBindTexture(GL_TEXTURE_2D, texture);

    if(direction==2 && what!=1)//�G�ƃ{�X�͕����]������
        PutSprite_RE(x,y,size,size);//�摜���]
    else
        PutSprite(x,y,size,size);
}
void PutMagic(int n)//�c�薂�@�g�p�񐔕\��
{
    int x,y;

    x=0;
    y=Char_Size*(n-1);
    
    glBindTexture(GL_TEXTURE_2D, item_texture[n-1]);//�c��g�p�񐔂̔w�i
    PutSprite(x,y,Char_Size,Char_Size);
    glBindTexture(GL_TEXTURE_2D, num_texture[attack[n].limit]);//�c��g�p��
    PutSprite(x,y,Char_Size,Char_Size);
}
void PutLife()//�{�X�̎c�胉�C�t�\��
{
    int x,y,i;

    x=WindowWidth;
    y=0;
    
    for(i=1;i<=boss.life;i++)//�c�胉�C�t�̕��n�[�g��\��
    {
        glBindTexture(GL_TEXTURE_2D, life_texture);//�c��g�p�񐔂̔w�i
        if(i<=10)
            PutSprite(x-Life_Size*i,y,Life_Size,Life_Size);//1�i��
        else
            PutSprite(x-Life_Size*(i-10),y+Life_Size,Life_Size,Life_Size);//2�i��
    }
}
void PutSprite(int x,int y,int width,int height)//�摜�\��t��
{
    //�摜�\��t��
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
void PutSprite_RE(int x,int y,int width,int height)//���]���ĉ摜�\��t��
{
    glPushMatrix();
    glEnable(GL_TEXTURE_2D);

        glBegin(GL_QUADS);
        //+++++++++++++++++++�摜�̉������ɔ��]�����Ă���+++++++++++++++
        glTexCoord2f(0, 1); glVertex2i(x + width , y + height);
        glTexCoord2f(0, 0); glVertex2i(x + width , y);
        glTexCoord2f(1, 0); glVertex2i(x , y);
        glTexCoord2f(1, 1); glVertex2i(x , y + height);
        glEnd();
    glDisable(GL_TEXTURE_2D);
    glPopMatrix();
}

void Attack(int what)//�U������
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
        if(attack[what].t==0)//����������
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
            if(what==3)//ice�̏ꍇ�㏸����
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
        if(attack[what].t==0)//����������
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
void Attack_teki(int n)//�G�̍U������
{
    float temp;


    if(teki_attack[n].t==0)//����������
    {
        teki_attack[n].y_sum=teki[n].y;

        if(teki[n].direction==2)//�E����
        {
            teki_attack[n].x_sum=teki[n].x+Char_Size;
            teki_attack[n].direction=2;
        }
        else//������
        {
            teki_attack[n].x_sum=teki[n].x-Char_Size;
            teki_attack[n].direction=3;
        }

        if(n%4==2)//�f�[����
        {
            temp=atan((float)(y_char-teki_attack[n].y_sum)/(float)(x_char-teki_attack[n].x_sum));//�p�x�����߂�
            teki_attack[n].x_move=((float)(x_char-teki_attack[n].x_sum))/50;//x��50��ړ��ŃL�����ɓ��B
            teki_attack[n].y_move=tan(temp)*teki_attack[n].x_move;//y�̈ړ���
        }
        else if(n%4==3)//�E�B�U�[�h
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
        //�ړ����x,y���v�Z
            teki_attack[n].x_sum+=teki_attack[n].x_move;
            teki_attack[n].y_sum+=teki_attack[n].y_move;
    }
    //x,y�̈ʒu����
    teki_attack[n].x=(int)teki_attack[n].x_sum;
    teki_attack[n].y=(int)teki_attack[n].y_sum;

    Anime(-1,n,0);//�A�j����\��
    teki_attack[n].t++;
    
    if(teki_attack[n].t>=60)
    {
        teki_attack[n].t=0;
        teki_attack[n].flag=0;
    }
}
void Attack_boss(int n)//�{�X�̍U������
{
    int i;

    if(boss_attack[n].t==0)//����������
    {
        if(n==0)//�_�[�N�T���_�[�̏ꍇ
        {
            boss_attack[n].x[0]=x_char;boss_attack[n].y[0]=0;//�o���ʒu
            boss_attack[n].x[1]=x_char-Char_Size;boss_attack[n].y[1]=0;
            boss_attack[n].x[2]=x_char+Char_Size;boss_attack[n].y[2]=0;
            boss_attack[n].direction=boss.direction;
            boss_attack[n].x_move[0]=0;boss_attack[n].y_move[0]=0;//�ړ���
            boss_attack[n].x_move[1]=0;boss_attack[n].y_move[1]=0;
            boss_attack[n].x_move[2]=0;boss_attack[n].y_move[2]=0;
        }
        else if(n==1 || n==2)//�u���[�t�@�C�A�ƃE�H�[�^�[�o���[���̏ꍇ
        {
            boss_attack[n].y[0]=boss.y+Char_Size;//�o���ʒu
            boss_attack[n].y[1]=boss.y+Char_Size;
            boss_attack[n].y[2]=boss.y+Char_Size;

            if(boss.direction==2)//�E����
            {
                boss_attack[n].x[0]=boss.x+Boss_Size;//�o���ʒu
                boss_attack[n].x[1]=boss.x+Boss_Size;
                boss_attack[n].x[2]=boss.x+Boss_Size;
                boss_attack[n].direction=2;
                boss_attack[n].x_move[0]=Step;//�ړ���
                boss_attack[n].x_move[1]=Step;
                boss_attack[n].x_move[2]=Step;
            }
            else//������
            {
                boss_attack[n].x[0]=boss.x-Char_Size;//�o���ʒu
                boss_attack[n].x[1]=boss.x-Char_Size;
                boss_attack[n].x[2]=boss.x-Char_Size;
                boss_attack[n].direction=3;
                boss_attack[n].x_move[0]=-1*Step;//�ړ���
                boss_attack[n].x_move[1]=-1*Step;
                boss_attack[n].x_move[2]=-1*Step;
            }
            boss_attack[n].y_move[0]=0;
            boss_attack[n].y_move[1]=0;
            boss_attack[n].y_move[2]=0;
            if(n==2)//�E�H�[�^�[�o���[���̏ꍇ��1�ڂ̐��ʂ̂ݓ����n�߂�
            {
                boss_attack[n].x_move[1]=0;
                boss_attack[n].x_move[2]=0;
            }
        }
        else//���e�I�̏ꍇ
        {//�o���ʒu�̓����_��
            boss_attack[n].x[0]=Char_Size*79 + Char_Size*(int)(20*(random()/(double)(RAND_MAX+1.0)));boss_attack[n].y[0]=0;
            boss_attack[n].x[1]=Char_Size*79 + Char_Size*(int)(20*(random()/(double)(RAND_MAX+1.0)));boss_attack[n].y[1]=0;
            boss_attack[n].x[2]=Char_Size*79 + Char_Size*(int)(20*(random()/(double)(RAND_MAX+1.0)));boss_attack[n].y[2]=0;
            boss_attack[n].direction=boss.direction;
            boss_attack[n].x_move[0]=0;boss_attack[n].y_move[0]=Step*4;//�ړ���
            boss_attack[n].x_move[1]=0;boss_attack[n].y_move[1]=Step*4;
            boss_attack[n].x_move[2]=0;boss_attack[n].y_move[2]=Step*4;
        }
    }
    else
    {
        if(boss_attack[n].t==6 && n==0)//�_�[�N�T���_�[�����J�n
        {
            boss_attack[n].y_move[0]=Step*3;
            boss_attack[n].y_move[1]=Step*3;
            boss_attack[n].y_move[2]=Step*3;
        }
        else if(boss_attack[n].t==20 && n==1)//�u���[�t�@�C�A�O���ω�
        {
            boss_attack[n].y_move[0]=0;//3�����ɔ��
            boss_attack[n].y_move[1]=-1*Step;
            boss_attack[n].y_move[2]=-1*Step;boss_attack[n].x_move[2]=0;
        }
        else if(n==2)//�E�H�[�^�[�o���[���O���ω�
        {
            switch(boss_attack[n].t)//���ԍ���2���ڂ�3���ڂ������o��
            {
                case 10 : boss_attack[n].x_move[1]=Step;if(boss_attack[n].direction==3)boss_attack[n].x_move[1]*=-1;break;
                case 20 : boss_attack[n].x_move[2]=Step;if(boss_attack[n].direction==3)boss_attack[n].x_move[2]*=-1;break;
            };
        }
        //�ړ����x,y���v�Z
        boss_attack[n].x[0]+=boss_attack[n].x_move[0];boss_attack[n].y[0]+=boss_attack[n].y_move[0];
        boss_attack[n].x[1]+=boss_attack[n].x_move[1];boss_attack[n].y[1]+=boss_attack[n].y_move[1];
        boss_attack[n].x[2]+=boss_attack[n].x_move[2];boss_attack[n].y[2]+=boss_attack[n].y_move[2];
    }

    for(i=0;i<3;i++)
        Anime(-2,n,i);//�A�j����\��
    boss_attack[n].t++;
    
    if(boss_attack[n].t>=90)
    {
        boss_attack[n].t=0;
        boss_attack[n].flag=0;
    }
}
void Anime(int what,int teki_n,int boss_n)//�U���Ȃǂ̃A�j��
{
    int x,y,tex,t_tmp,attack_direction;
    GLuint texture;

    if(x_char>=WindowWidth/2)//�X�N���[���͂��߂̏ꏊ
        x=attack[what].x-x_char+WindowWidth/2;
    else
        x=attack[what].x;

    if(what>=0)//�L�����̍U��
    {
        if(x_char >= Char_Size*(MapWidth-11) || right_end_flag==1)//�X�N���[���I���̏ꏊ
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
        else//�t�@�C�A�A�A�C�X�A�T���_�[
        {
            if(what==3)//�T���_�[�p
                t_tmp=attack[what].t%2+1;
            else//�t�@�C�A�ƃA�C�X�p
                t_tmp=attack[what].t%3;

            if(attack[what].t<5 && what==3)//�T���_�[�̏ꍇ�͍ŏ������������摜
                tex=0;
            else
                tex=t_tmp;
        }
    }
    else if(what==-1)//�G�̍U��
    {
        if(x_char>=WindowWidth/2)//�X�N���[���͂��߂̏ꏊ
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
        if(x_char >= Char_Size*(MapWidth-11) || right_end_flag==1)//�X�N���[���I���̏ꏊ
            x=teki_attack[teki_n].x-Char_Size*(MapWidth-21);
    }
    else if(what==-2)//�{�X�̍U��
    {
        if(x_char>=WindowWidth/2)//�X�N���[���͂��߂̏ꏊ
            x=boss_attack[teki_n].x[boss_n]-x_char+WindowWidth/2;
        else
            x=boss_attack[teki_n].x[boss_n];
        
        if(teki_n==0)//�_�[�N�T���_�[�p
            t_tmp=boss_attack[teki_n].t%2+1;
        else//�u���[�t�@�C�A�ƃE�H�[�^�[�o���[���ƃ��e�I�p
            t_tmp=boss_attack[teki_n].t%3;

        if(boss_attack[teki_n].t<5 && teki_n==0)//�_�[�N�T���_�[�̏ꍇ�͍ŏ������������摜
            tex=0;
        else if(t_tmp==0 && teki_n==1)//�u���[�t�@�C�A�p
            tex=0;
        else
            tex=t_tmp;
        if(x_char >= Char_Size*(MapWidth-11) || right_end_flag==1)//�X�N���[���I���̏ꏊ
            x=boss_attack[teki_n].x[boss_n]-Char_Size*(MapWidth-21);
    }

    if(what>=0)//�L�����̍U���̂Ƃ�
    {
        attack_direction=attack[what].direction;
        texture=attack[what].texture[tex];
        y=attack[what].y;
    }
    else if(what==-1)//�G�̍U���̂Ƃ�
    {
        attack_direction=teki_attack[teki_n].direction;
        texture=teki_attack[teki_n].texture[tex];
        y=teki_attack[teki_n].y;
    }
    else if(what==-2)//�{�X�̍U���̂Ƃ�
    {
        attack_direction=boss_attack[teki_n].direction;
        texture=boss_attack[teki_n].texture[tex];
        y=boss_attack[teki_n].y[boss_n];
    }

    glBindTexture(GL_TEXTURE_2D,texture);

    if(attack_direction==2)//�E����
    {
        PutSprite(x,y,Char_Size,Char_Size);
    }
    else//������
    {
        PutSprite_RE(x,y,Char_Size,Char_Size);//�摜���]
    }
}
void Teki_hantei(void)//++++++++++++++++++�G�Ƃ̏Փ˂�U���A�A�C�e���̔���Ȃ�+++++++++++
{
    int i,j;
    
    //++++++++++++++++++++++++++++++++�G�ƃA�C�e���̏���+++++++����������₱����+++++
    for(i=0;i<Teki_NUM;i++)//��ʊO�̓G�͕`�悵�����Ȃ����߂̏���
    {
        //+++++++�G�̏���+++++++++++++++++++++++++++++++++++++++++++++++++
        if(//��ʓ��ɓG������
            (teki[i].x<WindowWidth && x_char<WindowWidth/2) ||
            (abs(teki[i].x-x_char)<WindowWidth/2) ||
            (teki[i].x>Char_Size*(MapWidth-21) && (x_char>Char_Size*(MapWidth-11) || right_end_flag==1))
        )
        {
            if(teki[i].flag==0 && teki[i].out_flag==1)//�O�͉�ʊO�ɂ���
            {
                teki[i].flag=1;
                teki[i].out_flag=0;
            }
        }
        else//�G�͉�ʊO�ɂ���
        {
            teki[i].flag=0;
            teki[i].out_flag=1;
        }
        //+++++++++++++++++++++�G�̍U��+++++++++++++++++
        if((abs(x_char-teki_attack[i].x)<Char_Size) && (abs(y_char-teki_attack[i].y)<Char_Size) && teki_attack[i].flag==1)
        {
            //+++++++++�G�̍U����H�����+++++++++++++++++++++++
            Die();
        }

        if(teki[i].flag==1)//�������Ă���G�݈̂ړ�
        {
            if((abs(x_char-teki[i].x)<Char_Size) && (abs(y_char-teki[i].y)<Char_Size))
            {
                //++++++++++++++�G�ƂԂ�����+++++++++++++++
                Die();
            }
            for(j=0;j<Attack_NUM;j++)
            {
                if((abs(attack[j].x-teki[i].x)<Char_Size) && (abs(attack[j].y-teki[i].y)<Char_Size) && attack[j].flag==1)
                {
                    if(j==1 || j==2)//FIRE,ICE�̏ꍇ
                    {
                        attack[j].flag=0;//�΂��X��������
                        attack[j].t=0;
                    }

                    //+++++++++�G�ɍU������++++++++++++++++++
                    teki[i].flag=0;

                    if(item[i].flag==0 && (int)(5*(random()/(double)(RAND_MAX+1.0)))==0)//�����_���ŃA�C�e���𗎂Ƃ�
                    {
                        item[i].flag=1;//�A�C�e����\�����邩�ǂ����̃t���O
                        item[i].appear_flag=1;//�A�C�e�������݂��邩���Ȃ����̃t���O
                        item[i].x=teki[i].x;item[i].y=teki[i].y;
                    }
                }
            }

            Kabe_hantei(1,i);//teki�̔���
        }
        //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

        //++++++++++++++++++++�A�C�e���̏���++++++++++++++++++++++++++++++++++
        if(//��ʓ��ɃA�C�e��������
            (item[i].x<WindowWidth && x_char<WindowWidth/2) ||
            (abs(item[i].x-x_char)<WindowWidth/2) ||
            (item[i].x>Char_Size*(MapWidth-21) && x_char>Char_Size*(MapWidth-11))
        )
        {
            if(item[i].flag==0 && item[i].out_flag==1 && item[i].appear_flag==1)//�O�͉�ʊO�ɂ�����
            {
                item[i].flag=1;
                item[i].out_flag=0;
            }
        }
        else//�A�C�e���͉�ʊO�ɂ���
        {
            item[i].flag=0;
            item[i].out_flag=1;
        }
        if(item[i].flag==1)//���݂��Ă���A�C�e���̂ݔ���
        {
            if((abs(x_char-item[i].x)<Char_Size) && (abs(y_char-item[i].y)<Char_Size))
            {
                //+++++++++++++++++++�A�C�e�����Q�b�g����++++++++++++++++++
                if(attack[item[i].kind+1].limit<3)//�ő�X�g�b�N3
                    attack[item[i].kind+1].limit++;//���@�̎g�p�񐔑���
                item[i].flag=0;//�\�����邩���Ȃ����̃t���O
                item[i].appear_flag=0;//�A�C�e�������݂��邩���Ȃ����̃t���O
            }
        }
        //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    }
}
void Boss_hantei(void)//+++++++++++++++++++++�{�X�̏���++++++++++++++++
{
    int i,j;
    
    if(boss.nodamage_t!=0)
        boss.nodamage_t--;

    for(i=0;i<Boss_Attack_NUM;i++)
    {
        for(j=0;j<3;j++)
        {
            //+++++++++++++++++++++�{�X�̍U��+++++++++++++++++
            if((abs(x_char-boss_attack[i].x[j])<Char_Size) && (abs(y_char-boss_attack[i].y[j])<Char_Size) && boss_attack[i].flag==1)
            {
                //+++++++++++++++�{�X�̍U����H�����++++++++++++++++
                Die();
            }
        }
    }

    if(//+++++++++++++++++++++++�{�X�Ƃ̏Փ˔���+++++++�T�C�Y���Ⴄ���߂�₱����++++++
        (
            (x_char<=boss.x && (abs(x_char-boss.x)<Char_Size))//�L�����̂ق������ɂ���ꍇ
            ||
            (x_char>boss.x && (abs(x_char-boss.x)<Boss_Size))//�L�����̂ق����E�ɂ���ꍇ
        )
        &&
        (
            (y_char<=boss.y && (abs(y_char-boss.y)<Char_Size))//�L�����̂ق�����ɂ���ꍇ
            ||
            (y_char>boss.y && (abs(y_char-boss.y)<Boss_Size))//�L�����̂ق�����ɂ���ꍇ
        )
      )
    {
        //++++++�{�X�ƂԂ�����+++++++++++++++++++++++++++++++++++++
        Die();
    }
    for(j=0;j<Attack_NUM;j++)
    {
        if(//+++++++++++++++++++++++�{�X�ւ̍U������+++++++�T�C�Y���Ⴄ���߂�₱����++++++
            (
                (attack[j].x<=boss.x && (abs(attack[j].x-boss.x)<Char_Size))//�U���̂ق������ɂ���ꍇ
                ||
                (attack[j].x>boss.x && (abs(attack[j].x-boss.x)<Boss_Size))//�U���̂ق����E�ɂ���ꍇ
            )
            &&
            (
                (attack[j].y<=boss.y && (abs(attack[j].y-boss.y)<Char_Size))//�U���̂ق�����ɂ���ꍇ
                ||
                (attack[j].y>boss.y && (abs(attack[j].y-boss.y)<Boss_Size))//�U���̂ق������ɂ���ꍇ
            )
            &&
            attack[j].flag==1//�U�����J��o���Ă���
            &&
            boss.nodamage_t==0//���G���Ԃł͂Ȃ�
            &&
            die_t==0//��l��������ł��鎞�ł͂Ȃ�
          )
        {
            //++++++++++++++�{�X�ɍU������++++++++++++++++++++++++++
            attack[j].flag=0;//�U����������=�A���ōU���ł��Ȃ�
            attack[j].t=0;
            if(j==0 || j==3)//�������̏ꍇ��2�_���[�W
                boss.life-=2;
            else
                boss.life--;
            if(boss.life<=0)//�{�X��|����
            {
                boss.flag=0;
                init();
                clear_flag=1;stage_start=0;
                Game_Clear();//�Q�[���N���A��̃V�[���J�n
            }
            boss.nodamage_t=30;//�{�X�̖��G���Ԑݒ�(�A���ōU���ł��Ȃ�)
        }
    }

    Kabe_hantei(2,i);//boss�̔���
}
void Kabe_hantei(int who,int n)    //+++++++++++�ړ��悪�ǂ��ǂ�������+++++++++++++++++
{
    int cx,cy;
    int x,y;
    float t;
    int cursor,rand_tmp,teki_temp;//teki_temp�͓G�ɂ���ĕ�����ς��邽��
    int step_temp;//�G�ƃL�����̕�����ς���
    int boss_n;//�{�X�̍U�����@�I��

    if(who==0)//�L�����ړ��̏ꍇ
    {
        x=x_char;
        y=y_char;
        cursor=cursor_flag;
        step_temp=Step;
        t=jump_t;

        cx=x/Char_Size;
        cy=y/Char_Size;

        if(jump_flag==1)//�W�����v
        {
            cy=(y-1)/Char_Size;
            if(
                (map[cy][cx]=='C' || (x%Char_Size!=0 && map[cy][cx+1]=='C'))
                ||
                (JumpHeight-9.8*t<=0)
            )//�ǂɓ��������ꍇ���W�����v�͂��Ȃ��Ȃ����ꍇ
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
                    y+=Char_Size-y%Char_Size;//�u���b�N�ɓ������܂�Ȃ��悤��
                if(JumpHeight-9.8*t<=0)//���x���t�����ɂȂ�
                {
                    fall_flag=1;
                    jump_flag=0;t=0;y_tempchar=y;
                }
            }
            else
            {//��ʏ�ɓ��������ꍇ
                fall_flag=1;
                jump_flag=0;t=0;y_tempchar=y;
            }
        }
        else if(!(map[cy+1][cx]=='C' || (x%Char_Size!=0 && map[cy+1][cx+1]=='C')))//������
        {
            if(fall_flag==0)//���ړ��ŗ����n�߂��Ƃ�
                y_tempchar=y;
            fall_flag=1;
            if(y+Char_Size+Step<WindowHeight)
            {
                t+=0.24;
                y=y_tempchar + 9.8*t*t/2;//��������
                cy=y/Char_Size;
                if(map[cy+1][cx]=='C' || (x%Char_Size!=0 && map[cy+1][cx+1]=='C'))//���n
                    y-=y%32;//�u���b�N�ɖ��܂�Ȃ��悤��
            }
            else//�������Ƃ�
            {
                cursor=0;t=0;
            }
        }
        else//�W�����v�I��
        {
            fall_flag=0;t=0;
        }
    }
    else if(who==1)//�G�̏ꍇ
    {
        x=teki[n].x;
        y=teki[n].y;
        if(n%4==0)//�X���C��
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
            if(n%4==2)//��ԓG
                rand_tmp=5;
            else//�����G
                rand_tmp=3;
            if(cursor!=teki[n].direction || n%4==2)//�����G�ɂ͈ړ��������A��ԓG�ɂ͂Ȃ�
            {
                switch((int)(rand_tmp * (random()/(double)(RAND_MAX+1.0))))
                {
                    case 0 : cursor=0;teki[n].direction=(int)(2 * (random()/(double)(RAND_MAX+1.0)))+2;break;
                    case 1 : cursor=3;teki[n].direction=3;break;//2,3�͉������Ȃ̂ŕ֋X��
                    case 2 : cursor=2;teki[n].direction=2;break;//3��
                    case 3 : cursor=1;break;//1������ւ���Ă���
                    case 4 : cursor=4;break;
                };
            }

            if(n%4>=2)//��ԓG�Ɩ��@�g���͍U������
            {
                if(teki_attack[n].flag==0 && cursor==0)//�G���~�܂�ƍU��
                {
                    if((teki[n].direction==2 && teki[n].x<x_char)//�����Ă�������ɃL������������
                     ||
                     (teki[n].direction==3 && teki[n].x>x_char))
                        teki_attack[n].flag=1;
                }
            }
        }
        teki[n].t++;
    }
    else if(who==2)//�{�X�̏ꍇ
    {
        x=boss.x;
        y=boss.y;

        step_temp=Step-2;//�������x
        teki_temp=40;//�����]���Ȃǂ̎���

        cursor=boss.cursor;

        if(boss.t==teki_temp)
        {
            boss.walk_flag=boss.walk_flag^1;
            boss.t=0;
            rand_tmp=2;//�����{�X�ɂ͈ړ�����

            switch((int)(rand_tmp * (random()/(double)(RAND_MAX+1.0))))
            {
                case 0 : cursor=0;break;
                case 1 : cursor=boss.direction;break;//�����Ă�������֐i��
            };
            if(x_char<x)//�L���������ɂ���ꍇ
                boss.direction=3;
            else        //�L�������E�ɂ���ꍇ
                boss.direction=2;

            //++++++++++++�{�X�̍U��++++++++++++++++++++++++++++++++
            boss_n=(int)(Boss_Attack_NUM * (random()/(double)(RAND_MAX+1.0)));
            if(boss_attack[0].flag==0 && boss_attack[1].flag==0 && boss_attack[2].flag==0 && boss_attack[3].flag==0 && cursor==0)//�~�܂�ƍU��
            {
                boss_attack[boss_n].flag=1;
            }
        }
        boss.t++;
    }

    cx=x/Char_Size;
    cy=y/Char_Size;
    switch(cursor)//�ړ���ɕǂ����邩�ǂ���
    {
        case 1://��
            cy=(y-1)/Char_Size;
            if(map[cy][cx]=='C' || (x%Char_Size!=0 && map[cy][cx+1]=='C'))
            {
                cursor=0;
            }
            break;
        case 2://��
            if(who==2)//�{�X�̏ꍇ
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
                
                if(who==1 && n%4!=2)//�����G�̏ꍇ�A�i�����痎���Ȃ��悤��
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
        case 3://��
            cx=(x-1)/Char_Size;
            if(map[cy][cx]=='C' || (y%Char_Size!=0 && map[cy+1][cx]=='C'))
            {
                cursor=0;
            }
            if(who==1 && n%4!=2)//�����G�̏ꍇ�A�i�����痎���Ȃ��悤��
            {
                cx=(x-Char_Size)/Char_Size;
                if(!(map[cy+1][cx]=='C' || (x%Char_Size!=0 && map[cy+1][cx+1]=='C')))
                {
                    cursor=0;
                    teki[n].direction=2;
                }            
            }
            break;
        case 4://��
            if(map[cy+1][cx]=='C' || (x%Char_Size!=0 && map[cy+1][cx+1]=='C'))
            {
                cursor=0;
            }
            break;
        case 0://�L�����N�^�[��p(�ǂ��z����u�ԗp)
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

    switch(cursor)//��ʊO�łȂ���Έړ�
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
void Die(void)//���S��̏���
{
    if(die_t==0)//���S�A�j���J�n
    {
        die_t=120;
        cursor_flag=0;cursor_key=0;
    }
        
    die_t--;
    if(die_t==0)//���S�I���
    {
        Stage_Change();
    }
}
void Stage_Change(void)//�X�e�[�W��ς���
{
    int i;
    char file_name[20];
        
    init();//������
    
    stage_start=0;//�X�e�[�W���J�n����Ȃ�
    pause_flag=1;

    if(stage==0)//�^�C�g�����
    {
        //++++++++++++�^�C�g���`��++++++++++++++++++++++++
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
    
        //+++++++++++�}�b�v�摜�ǂݍ���++++++++++++++++++++++++++++
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
        
        //++++++++++++�G�̏����ݒ�+++++++++++++++++++++++
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
        
        sleep(1);//���[�f�B���O��ʂ̃E�F�C�g
        
        //++++++++++++STAGE�J�n�̔w�i++++++++++++++++++++++++
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

        //+++++++++++�}�b�v�摜�ǂݍ���++++++++++++++++++++++++++++
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
        
        //++++++++++++�G�̏����ݒ�+++++++++++++++++++++++
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
        //++++++�ȉ��̓G�̓Q�[���ɂ͎Q�����Ȃ�+++++++++++++++++
        teki[20].x=Char_Size*150;teki[20].y=Char_Size*12;
        teki[21].x=Char_Size*150;teki[21].y=Char_Size*12;
        teki[22].x=Char_Size*150;teki[22].y=Char_Size*12;
        teki[23].x=Char_Size*150;teki[23].y=Char_Size*12;
        teki[24].x=Char_Size*150;teki[24].y=Char_Size*12;
        boss.x=Char_Size*96;boss.y=Char_Size*11;boss.life=20;
        
        sleep(1);//���[�f�B���O��ʂ̃E�F�C�g
        
        //++++++++++++STAGE�J�n�̔w�i++++++++++++++++++++++++
        glClear(GL_COLOR_BUFFER_BIT);
        glBindTexture(GL_TEXTURE_2D,scene_texture[3]);
        PutSprite(0,0,WindowWidth,WindowHeight);
        glFlush();
        glutSwapBuffers();
    }
}
void Game_Clear(void)
{
    if((z_key==1 && z_up==0) || clear_scene==4)//�������ςȂ��ł��i�܂Ȃ��Bz���������Ƃ��ƃN���A����
    {
        z_key=0;
        
        if(clear_scene==Scene_NUM)//Fin�܂ŕ\������
        {
            stage=0;
            Stage_Change();
            return;
        }
        
        //++++++++++++�^�C�g���`��++++++++++++++++++++++++
        glClear(GL_COLOR_BUFFER_BIT);
        glBindTexture(GL_TEXTURE_2D,scene_texture[clear_scene]);
        PutSprite(0,0,WindowWidth,WindowHeight);
        glFlush();
        glutSwapBuffers();
        
        clear_scene++;
        sleep(1);
    }
}

void Keyboard(char  key,int x,int y)//�L�[����
{
    if(die_t!=0)//���S��
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
    else if(key=='z' && z_up==1)//�V�[������Z��������1��ʐi��
    {
        z_key=1;z_up=0;
        if(stage_start==0 && clear_flag==0)
        {
            stage_start=1;
            pause_flag=0;
            if(stage==0)//�Q�[���J�n
            {
                stage=1;
                Stage_Change();
            }
        }
    }
    if(key==32 && stage_start!=0 && stage!=0)//SPACE��ASCII��32
    {//�Q�[�����Ƀ|�[�Y
        if(pause_key==0)
        {
            pause_flag^=1;pause_key=1;
        }
    }
    else if(key==27)
    {
        exit(1);//�Q�[���I��
    }
}
void KeyboardUp(char key,int x,int y)//�L�[UP����
{
    if(key==32)//�|�[�Y����
        pause_key=0;
    if(key=='z' && stage_start==0)//�V�[����
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
void SpecialKey(int key,int x,int y)//�L�[����
{
    if(die_t!=0)//���S��
        return;
    switch(key)//�l�Ԃ͏�Ɖ��ɂ͈ړ��ł��Ȃ����A�G�͂ł��邽��
    {//cursor_flag��2��3
        case GLUT_KEY_RIGHT:cursor_flag=2;direction=2;cursor_key=2;break;
        case GLUT_KEY_LEFT:cursor_flag=3;direction=3;cursor_key=3;break;

        default:break;
    }
}
void SpecialUpKey(int key,int x,int y)//�L�[UP����
{
    switch(key)
    {
        case GLUT_KEY_RIGHT:if(cursor_flag==2 || cursor_flag==0){cursor_flag=0;cursor_key=0;}break;
        case GLUT_KEY_LEFT:if(cursor_flag==3 || cursor_flag==0){cursor_flag=0;cursor_key=0;}break;

        default:break;
    }
}
