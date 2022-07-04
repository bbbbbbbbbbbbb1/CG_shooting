#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <time.h>
#include <gl/glut.h>
#define _USE_MATH_DEFINES
#include <math.h>

#define PLSHOT_NUM 10 //�v���C���[�̒e��
#define PLSHOT_INTERVAL 5 //�v���C���[���e��A�����Č��Ă�Ԋu
#define PLDAMAGED_TIME 8 //�v���C���[���_���[�W���󂯂��ېF���ς������ԂƂȂ鎞�Ԃ̒���
#define OBSTACLE_NUM 30 //��Q���̐�
#define OBSTACLE_HP 5 //��Q���̏���HP
#define OBSTACLE_INTERVAL 30 //��Q�����o�Ă���Ԋu
#define ENEMY_HP 30 //�G�̏���HP
#define ENSHOT_INTERVAL 20 //�G���e�����Ԋu
#define ENSHOT_GRNUM 5 //�G�̒e�̃O���[�v��
#define ENSHOT_NUM 9 //�G�̒e��1�O���[�v������̐�
#define STRSIZE 256 //str�̍ő�̒���

typedef struct {	//���W�Ƒ傫���A��]�p�A�\���L����ۑ�����\����
    float x;//x���W
    float y;//y���W
    float z;//z���W
    double size;//�傫��
    int theta;//��]�p
    int appear;//���̍\���̂ɑΉ����镨�̂���ʏ�ɕ\������Ă���Ƃ�1�ƂȂ�ϐ�(�v���C���[�͖��G���Ԃł���Ԃ��̕ϐ���0�ƂȂ�)
} coordinate;

typedef struct {	//HP�ƍ��W�A�傫���A��]�p�A�\���L����ۑ�����\����
    int hp;//HP
    coordinate c;
} HPandCoord;

coordinate player, plshot[PLSHOT_NUM], enshotgr[ENSHOT_GRNUM], enshot[ENSHOT_GRNUM * ENSHOT_NUM];//�v���C���[�A�v���C���[�̒e�A�G�̒e�̃O���[�v�A�G�̒e(���ۂ̒e)�̍��W�Ƒ傫���A��]�p�A�\���L����ێ�����\����
HPandCoord obstacle[OBSTACLE_NUM], enemy;//��Q���ƓG��HP�ƍ��W�A�傫���A��]�p�A�\���L����ۑ�����\����

enum keys {
    W_KEY, A_KEY, S_KEY, D_KEY, SPACE_KEY
};

int usingkeys[5] = { 0 }; //�e�L�[�������ꂽ��Ԃł���Ƃ�1�ƂȂ�ϐ�, �C���f�b�N�X��enum keys�ɑΉ�

int score = 0;//�Q�[���̃X�R�A
int plshotIdx = 0;//���ɔ��˂���v���C���[�̒e(�z��̗v�f)�̃C���f�b�N�X���i�[����ϐ�
int plshotTime = -1;//�v���C���[���e�����Ԋu�𐧌䂷�邽�߂̕ϐ�
int pldamagedTime = -1;//�v���C���[����Q�����邢�͓G�̒e�ƏՓ˂����ۈ�莞�ԐF��ς��邽�߂ɗp����ϐ�
int encoming = 0;//�G���o�������㏊���z���W�ɒ����܂ł̊�1�ƂȂ�A��������0�ƂȂ�ϐ�
int enshotgrIdx = 0;////���ɏo�Ă���G�̒e�̃O���[�v(�z��̗v�f)�̃C���f�b�N�X���i�[����ϐ�
int enshotTime = -1;//�G���e�����Ԋu�𐧌䂷�邽�߂̕ϐ�
int obIdx = 0;//���ɏo�Ă����Q��(�z��̗v�f)�̃C���f�b�N�X���i�[����ϐ�
int obTime = 0;//��Q�����o�ꂷ��Ԋu�𐧌䂷�邽�߂̕ϐ�
int gameEnd = 0;//�G(��Q���ł͂Ȃ�)���|���ꂽ��1�Ƃ���ϐ�
char str[STRSIZE];//��ʂɕ\�����镶������i�[����ϐ�

void printstr(char* str);//��ʂɕ������\������֐�
int collisionJudge(coordinate object1, coordinate object2);//�Փ˔�����s���֐�, object1��object2���ڐG(�Փ�)���Ă���Ƃ�1, �����łȂ��Ƃ�0��Ԃ�
void playerdamaged();//�v���C���[����Q�����邢�͓G�̒e�ƏՓ˂����ۂ̏������s���֐�
void enemydraw(int damaged);//�G�̕`����s���֐�

//------------ �e��ގ��p�����[�^--------------//
//����(������g�U������ʌ���ʒu)
GLfloat lightAmb[] = { 0.f, 0.f, 0.f, 1.f };
GLfloat lightDiff[] = { 1.f, 1.f, 1.f, 1.f };
GLfloat lightSpec[] = { 1.f, 1.f, 1.f, 1.f };
GLfloat lightPos[] = { 1.f, 1.f, 1.f, 0.f };//���s����

//�ގ� (������g�U������ʌ�����ʎw��) �̃p�����[�^�i���r�[�j
GLfloat rubyAmb[] = { 0.1745f, 0.01175f, 0.01175f, 1.f };
GLfloat rubyDiff[] = { 0.61424f, 0.04136f, 0.04136f, 1.f };
GLfloat rubySpec[] = { 0.727811f, 0.626959f, 0.626959f, 1.f };
GLfloat rubyShin = 76.8f;

//�ގ� (������g�U������ʌ�����ʎw��) �̃p�����[�^�i�^�[�R�C�Y�j
GLfloat turquoiseAmb[] = { 0.1f, 0.18725f, 0.1745f, 1.f };
GLfloat turquoiseDiff[] = { 0.396f, 0.74151f, 0.69102f, 1.f };
GLfloat turquoiseSpec[] = { 0.297254f, 0.30829f, 0.306678f, 1.f };
GLfloat turquoiseShin = 12.8f;

//�ގ� (������g�U������ʌ�����ʎw��) �̃p�����[�^�i�p�[���j
GLfloat pearlAmb[] = { 0.25f, 0.20725f, 0.20725f, 1.f };
GLfloat pearlDiff[] = { 1.0f, 0.829f, 0.829f, 1.f };
GLfloat pearlSpec[] = { 0.296648f, 0.296648f, 0.296648f, 1.f };
GLfloat pearlShin = 11.264f;

//�ގ� (������g�U������ʌ�����ʎw��) �̃p�����[�^�i�����v���X�`�b�N�j
GLfloat whiteplasticAmb[] = { 0.f, 0.f, 0.f, 1.f };
GLfloat whiteplasticDiff[] = { 0.55f, 0.55f, 0.55f, 1.f };
GLfloat whiteplasticSpec[] = { 0.70f, 0.70f, 0.70f, 1.f };
GLfloat whiteplasticShin = 32.f;

//�ގ� (������g�U������ʌ�����ʎw��) �̃p�����[�^�i���j�΁j
GLfloat obsidianAmb[] = { 0.05375f, 0.05f, 0.06625f, 1.f };
GLfloat obsidianDiff[] = { 0.18275f, 0.17f, 0.22525f, 1.f };
GLfloat obsidianSpec[] = { 0.332741f, 0.328634f, 0.346435f, 1.f };
GLfloat obsidianShin = 38.4f;

//�ގ� (������g�U������ʌ�����ʎw��) �̃p�����[�^�i���j
GLfloat goldAmb[] = { 0.24725f, 0.1995f, 0.0745f, 1.f };
GLfloat goldDiff[] = { 0.75164f, 0.60648f, 0.22648f, 1.f };
GLfloat goldSpec[] = { 0.628281f, 0.555802f, 0.366065f, 1.f };
GLfloat goldShin = 51.2f;

//--------- �e��R�[���o�b�N�֐�-------//
void display(void)
{
    static int angle = 0;

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    gluLookAt(0.0, 0.5, 4.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);

    //�����̈ʒu�ݒ�
    glLightfv(GL_LIGHT0, GL_POSITION, lightPos);

    //�L�[���͔��f
    if (player.y < 0.75f) {//�܂���ɍs����ꍇ
        if (usingkeys[W_KEY] == 1) { //'w'�L�[�����͂���Ă��鎞
            player.y += 0.02f;//��ɕ��s�ړ�
        }
    }
    if (player.x > -0.82f) {//�܂����ɍs����ꍇ
        if (usingkeys[A_KEY] == 1) { //'a'�L�[�����͂���Ă��鎞
            player.x -= 0.02f;//���ɕ��s�ړ�
        }
    }
    if (player.y > -0.32f) {//�܂����ɍs����ꍇ
        if (usingkeys[S_KEY] == 1) { //'s'�L�[�����͂���Ă��鎞
            player.y -= 0.02f;//���ɕ��s�ړ�
        }
    }
    if (player.x < 0.82f) {//�܂��E�ɍs����ꍇ
        if (usingkeys[D_KEY] == 1) { //'d'�L�[�����͂���Ă��鎞
            player.x += 0.02f;//�E�ɕ��s�ړ�
        }
    }
    if (plshotTime == -1 && usingkeys[SPACE_KEY] == 1) {//�v���C���[���e��łĂ��ԂŁA���X�y�[�X�L�[��������Ă���Ƃ�
        int plshotok = 1;//�܂��`�悳��Ă��Ȃ�plshot[]�̗v�f��1���Ȃ����0�ł���A1�ł������1�ł���Ƃ���ϐ�
        if (plshot[plshotIdx].appear == 1) {//����plshot[plshotIdx]�����ɕ`�撆�ł���΁A�܂��`�悳��Ă��Ȃ�plshot[]�̗v�f��T��
            plshotok = 0;//�܂��`�悳��Ă��Ȃ�plshot[]�̗v�f��1���Ȃ������Ƃ��p��0�Ƃ��Ă���
            for (int i = 0; i < PLSHOT_NUM; i++) {
                if (plshot[i].appear == 0) {//�܂��`�悳��Ă��Ȃ�plshot[]�̗v�f�����������ꍇ
                    plshotIdx = i;//plshotIdx���������v�f�̃C���f�b�N�X�̒l�ɂ���
                    plshotok = 1;//�܂��`�悳��Ă��Ȃ�plshot[]�̗v�f�����������̂�plshot��1�ɖ߂�
                }
            }
        }
        if (plshotok == 1) {//�܂��`�悳��Ă��Ȃ�plshot[]�̗v�f��1�ł�����΂��̗v�f(plshot[plshotIdx])�̕`����J�n����
            plshotTime++;//plshotTime�̌v���J�n
            plshot[plshotIdx].x = player.x;//�v���C���[������ʒu�����̒e�̏����ʒu�Ƃ���
            plshot[plshotIdx].y = player.y;
            plshot[plshotIdx].z = player.z;
            plshot[plshotIdx].appear = 1;//plshotIdx�ɑΉ�����v���C���[�̒e���o��������
        }
        plshotIdx++;//plshotIdx���X�V����
        if (plshotIdx >= PLSHOT_NUM) {//plshotIdx��plshot�̃C���f�b�N�X���𒴂����Ƃ�
            plshotIdx = 0;//plshotIdx������������
        }
    }

    //��Q���̕`��
    if (obTime >= 0) {//obTime��0�ȏ�̂Ƃ�obTime�̌v�����s��
        obTime++;
        if (obTime > OBSTACLE_INTERVAL) {//obTime�̌v�����n�܂��Ă������̊Ԋu���o�߂����Ƃ�
            obTime = 0;
            obstacle[obIdx].c.x = (float)(((((float)rand() / (float)RAND_MAX) * 2.0) - 1.0) * 0.82);//�����ʒu�̓v���C���[�̈ړ��\�͈͓��Ń����_���Ȉʒu�Ƃ���
            obstacle[obIdx].c.y = (float)((((float)rand() / (float)RAND_MAX) * (0.75 + 0.32)) - 0.32);
            obstacle[obIdx].c.z = -30.f;
            obstacle[obIdx].c.appear = 1;//obIdx�ɑΉ������Q�����o��������
            obIdx++;//obIdx���X�V����
            if (obIdx >= OBSTACLE_NUM) {//obIdx��obstacle�̃C���f�b�N�X���𒴂����Ƃ�
                obTime = -1;//obTime�̌v�����~����
            }
        }
    }
    int obappear = 0;//�`�悳����Q����1�ł�����Ƃ�1�ɂȂ�ϐ�
    for (int i = 0; i < OBSTACLE_NUM; i++) {
        if (obstacle[i].c.appear == 1) {
            obappear = 1;
            obstacle[i].c.z += 0.1f;
            if (obstacle[i].c.z >= 1.4f) {
                obstacle[i].c.appear = 0;
            }
            else {
                glPushMatrix();
                glTranslatef(obstacle[i].c.x, obstacle[i].c.y, obstacle[i].c.z);
                int obcollision = 0;//���̏�Q�����v���C���[�̒e�ƏՓ˂��Ă���Ƃ�1�Ƃ���ϐ�
                for (int j = 0; j < PLSHOT_NUM; j++) {
                    if (plshot[j].appear == 1) {//���̏�Q���ƏՓ˔�����s���͕̂`�悳��Ă���v���C���[�̒e�݂̂Ƃ���
                        if (collisionJudge(obstacle[i].c, plshot[j]) == 1) {//���̏�Q�����v���C���[�̒e�ƏՓ˂��Ă���Ƃ�
                            obcollision = 1;
                            plshot[j].appear = 0;//�Փ˂��Ă���v���C���[�̒e������
                            obstacle[i].hp--;//���̏�Q����HP��1���炷
                            glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, rubyAmb);//��u(���ɂ��̊֐����Ă΂��܂�)�F��ς���
                            glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, rubyDiff);
                            glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, rubySpec);
                            glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, rubyShin);
                            break;//����(���̊֐���1�x�̎��s��)�ɕ����̃v���C���[�̒e�������邱�Ƃ͂Ȃ��悤�ɂ���
                        }
                    }
                }
                if (obcollision == 0) {
                    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, turquoiseAmb);
                    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, turquoiseDiff);
                    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, turquoiseSpec);
                    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, turquoiseShin);
                }
                glutSolidCube(obstacle[i].c.size);//�`��
                glPopMatrix();
                if (obstacle[i].hp <= 0) {//���̏�Q����HP���Ȃ��Ȃ����Ƃ�
                    score += 10000;//�X�R�A�𑝂₷
                    obstacle[i].c.appear = 0;//���̏�Q��������
                }
            }
        }
    }
    if (obTime == -1 && obappear == 0 && encoming == 0 && enemy.c.appear == 0 && gameEnd == 0) {//�S�Ă̏�Q���̏o���Ə��ł��I����Ă���A�܂��G������Ă��Ȃ��ꍇ
        encoming = 1;//�G�̐ڋ߂��J�n����
    }

    //�G�̕`��
    if (encoming == 1) {//�G�������z���W�܂ňړ�������
        enemy.c.z += 0.1f;
        enemydraw(0);//�`��
        if (enemy.c.z >= -10.f) {//�G�������z���W�܂Œ�������G���o��������Ԃł���Ƃ���
            encoming = 0;
            enemy.c.appear = 1;
            enshotTime++;//enshotTime�̌v���J�n
        }
    }
    if (enemy.c.appear == 1) {
        enemy.c.y = 0.3f * (float)sin((((double)enemy.c.theta * 2.0) / 180.0) * M_PI);
        enemy.c.x = 0.8f * (float)sin((((double)enemy.c.theta / 180.0) * M_PI));
        int encollision = 0;//�G���v���C���[�̒e�ƏՓ˂��Ă���Ƃ�1�Ƃ���ϐ�
        for (int i = 0; i < PLSHOT_NUM; i++) {
            if (plshot[i].appear == 1) {//�G�ƏՓ˔�����s���͕̂`�悳��Ă���v���C���[�̒e�݂̂Ƃ���
                if (collisionJudge(enemy.c, plshot[i]) == 1) {//�G���v���C���[�̒e�ƏՓ˂��Ă���Ƃ�
                    encollision = 1;
                    plshot[i].appear = 0;//�Փ˂��Ă���v���C���[�̒e������
                    enemy.hp--;//�G��HP��1���炷
                    break;//����(���̊֐���1�x�̎��s��)�ɕ����̃v���C���[�̒e�������邱�Ƃ͂Ȃ��悤�ɂ���
                }
            }
        }
        enemydraw(encollision);//�`��
        if (enemy.hp <= 0) {//�G��HP���Ȃ��Ȃ����Ƃ�
            score += 100000;//�X�R�A�𑝂₷
            enemy.c.appear = 0;//�G������
            gameEnd = 1;//���ʉ�ʂ̕\���Ɉڂ�
        }
        enemy.c.theta = (enemy.c.theta + 4) % 360;
    }

    //�G�̒e�̕`��
    if (enshotTime >= 0) {//enshotTime��0�ȏ�̂Ƃ�enshotTime�̌v�����s��
        enshotTime++;
        if (enshotTime > ENSHOT_INTERVAL && enshotgr[enshotgrIdx].appear == 0) {//enshotTime�̌v�����n�܂��Ă������̊Ԋu���o�߂��Ă���A���o��(�`��)����Ă��Ȃ��G�̒e�̃O���[�v������Ƃ�
            enshotTime = 0;
            enshotgr[enshotgrIdx].x = enemy.c.x;//�G������ʒu�����̓G�̒e�̃O���[�v�̏����ʒu�Ƃ���
            enshotgr[enshotgrIdx].y = enemy.c.y;
            enshotgr[enshotgrIdx].z = enemy.c.z;
            enshotgr[enshotgrIdx].appear = 1;//enshotgrIdx�ɑΉ�����G�̒e�̃O���[�v���o��������
            for (int i = enshotgrIdx * ENSHOT_NUM; i < (enshotgrIdx + 1) * ENSHOT_NUM; i++) {//���̃O���[�v�ɑ�����G�̒e���o��������
                enshot[i].appear = 1;
            }
            enshotgrIdx++;//enshotgrIdx���X�V����
            if (enshotgrIdx >= ENSHOT_GRNUM) {//grIdx��enshotgr�̃C���f�b�N�X���𒴂����Ƃ�
                enshotgrIdx = 0;//enshotgrIdx������������
            }
        }
    }
    for (int i = 0; i < ENSHOT_GRNUM; i++) {
        if (enshotgr[i].appear == 1) {
            enshotgr[i].z += 0.1f;
            if (enshotgr[i].z >= 1.4f || enemy.c.appear == 0) {//�`��͈͊O�ɏo���Ƃ��ȊO�ɁA�G���|���ꂽ�Ƃ����G�̒e�̃O���[�v�y�ѓG�̒e�������������s��
                enshotgr[i].appear = 0;
                for (int j = i * ENSHOT_NUM; j < (i + 1) * ENSHOT_NUM; j++) {//���̃O���[�v�ɑ�����G�̒e������
                    enshot[j].appear = 0;
                }
            }
            else {
                enshotgr[i].theta = (enshotgr[i].theta + 5) % 360;
                for (int j = i * ENSHOT_NUM; j < (i + 1) * ENSHOT_NUM; j++) {//���̃O���[�v�ɑ�����G�̒e�̕`��
                    if (enshot[j].appear == 1) {
                        enshot[j].x = enshotgr[i].x + (0.4f * (float)cos((((int)(enshotgr[i].theta + ((360 / ENSHOT_NUM) * j)) % 360) / 180.0) * M_PI));
                        enshot[j].y = enshotgr[i].y + (0.4f * (float)sin((((int)(enshotgr[i].theta + ((360 / ENSHOT_NUM) * j)) % 360) / 180.0) * M_PI));
                        enshot[j].z = enshotgr[i].z;
                        glPushMatrix();
                        glTranslatef(enshot[j].x, enshot[j].y, enshot[j].z);
                        glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, pearlAmb);
                        glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, pearlDiff);
                        glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, pearlSpec);
                        glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, pearlShin);
                        glutSolidSphere(enshot[j].size, 10, 10);//�`��
                        glPopMatrix();
                    }
                }
            }
        }
    }

    //�v���C���[�̒e�̕`��
    if (plshotTime >= 0) {//plshotTime��0�ȏ�̂Ƃ�plshotTime�̌v�����s��
        plshotTime++;
        if (plshotTime > PLSHOT_INTERVAL) {//plshotTime�̌v�����n�܂��Ă������̊Ԋu���o�߂����Ƃ�
            plshotTime = -1;//plshotTime������������
        }
    }
    for (int i = 0; i < PLSHOT_NUM; i++) {
        if (plshot[i].appear == 1) {
            plshot[i].z -= 0.2f;
            if (plshot[i].z <= -30.f) {
                plshot[i].appear = 0;
            }
            else {
                glPushMatrix();
                glTranslatef(plshot[i].x, plshot[i].y, plshot[i].z);
                glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, goldAmb);
                glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, goldDiff);
                glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, goldSpec);
                glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, goldShin);
                glutSolidSphere(plshot[i].size, 10, 10);//�`��
                glPopMatrix();
            }
        }
    }

    //�v���C���[�̕`��
    glPushMatrix();
    glRotatef((float)player.theta, 0.f, 1.f, 0.f);
    glTranslatef(-player.z, player.y, player.x);
    for (int i = 0; i < OBSTACLE_NUM; i++) {
        if (obstacle[i].c.appear == 1) {//�v���C���[�ƏՓ˔�����s����Q���͕`�悳��Ă�����݂̂̂Ƃ���
            if (collisionJudge(player, obstacle[i].c) == 1) {//�v���C���[����Q���ƏՓ˂��Ă���Ƃ�
                obstacle[i].c.appear = 0;//�Փ˂��Ă����Q��������
                playerdamaged();
                break;//����(���̊֐���1�x�̎��s��)�ɕ����̏�Q���������邱�Ƃ͂Ȃ��悤�ɂ���
            }
        }
    }
    for (int i = 0; i < ENSHOT_GRNUM * ENSHOT_NUM; i++) {
        if (enshot[i].appear == 1) {//�v���C���[�ƏՓ˔�����s���G�̒e�͕`�悳��Ă�����݂̂̂Ƃ���
            if (collisionJudge(player, enshot[i]) == 1) {//�v���C���[���G�̒e�ƏՓ˂��Ă���Ƃ�
                enshot[i].appear = 0;//�Փ˂��Ă���G�̒e������
                playerdamaged();
                break;//����(���̊֐���1�x�̎��s��)�ɕ����̓G�̒e�������邱�Ƃ͂Ȃ��悤�ɂ���
            }
        }
    }
    if (pldamagedTime == -1) {
        glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, goldAmb);
        glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, goldDiff);
        glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, goldSpec);
        glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, goldShin);
    }
    else{//pldamagedTime��0�ȏ�̂Ƃ�plshotTime�̌v�����s��
        pldamagedTime++;
        glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, rubyAmb);//�v���C���[�̐F��ʏ펞�ƈقȂ�F�ɂ���
        glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, rubyDiff);
        glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, rubySpec);
        glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, rubyShin);
        if (pldamagedTime > PLDAMAGED_TIME) {//pldamagedTime�̌v�����n�܂��Ă������̎��Ԃ��o�߂����Ƃ�
            pldamagedTime = -1;//pldamagedTime������������
        }
    }
    glutSolidTeapot(player.size);//�`��
    glPopMatrix();

    //��ʏ�ւ̕�����\��
    glPushMatrix();
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, pearlAmb);
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, pearlDiff);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, pearlSpec);
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, pearlShin);
    glNormal3f(0.f, 0.f, 1.1f);
    if (gameEnd == 0) {//�ʏ��ʂ̕�����\��
        if (enemy.c.appear == 1) {
            sprintf(str, "ENEMY'S HP : %2d", enemy.hp);
            glRasterPos3f(-0.7f, -0.33f, 1.5f);
            printstr(str);
        }
        sprintf(str, "SCORE : %06d", score);
        glRasterPos3f(-0.7f, -0.4f, 1.5f);
        printstr(str);
        strcpy(str, "Move : 'w''a''s''d' key, Shot : space key");
        glRasterPos3f(-0.7f, -0.47f, 1.5f);
        printstr(str);
    }
    else {//���ʉ�ʂ̕�����\��
        strcpy(str, "GAME CLEAR!!");
        glRasterPos3f(-0.104f, 0.3f, 1.5f);
        printstr(str);
        strcpy(str, "Congratulations!");
        glRasterPos3f(-0.14f, 0.23f, 1.5f);
        printstr(str);
        sprintf(str, "YOUR FINAL SCORE : %06d", score);
        glRasterPos3f(-0.2162f, 0.16f, 1.5f);
        printstr(str);
        strcpy(str, "Exit this program : 'q' key");
        glRasterPos3f(-0.233f, 0.09f, 1.5f);
        printstr(str);
    }
    glPopMatrix();

    glutSwapBuffers();
}

void printstr(char* str)//��ʂɕ������\������֐�
{
    for (int i = 0; i < (signed)strlen(str); i++) {
        char ch = str[i];
        glutBitmapCharacter(GLUT_BITMAP_9_BY_15, ch);
    }
}

int collisionJudge(coordinate object1, coordinate object2)//�Փ˔�����s���֐�, object1��object2���ڐG(�Փ�)���Ă���Ƃ�1, �����łȂ��Ƃ�0��Ԃ�
{
    double distance;//2�̕��̂̍��W�Ԃ̋���(���S�Ԃ̋���)���i�[����ϐ�
    distance = sqrt((((double)object1.x - (double)object2.x) * ((double)object1.x - (double)object2.x))
        + (((double)object1.y - (double)object2.y) * ((double)object1.y - (double)object2.y))
        + (((double)object1.z - (double)object2.z) * ((double)object1.z - (double)object2.z)));//�����̌v�Z�����s����
    if (distance <= (object1.size + object2.size) / 2.0) {//2�̕��̂����ƌ��āAdistance�����a�̘a�ȉ��Ȃ�ڐG(�Փ�)���Ă���Ƃ���
        return 1;
    }

    return 0;
}

void playerdamaged()//�v���C���[����Q�����邢�͓G�̒e�ƏՓ˂����ۂ̏������s���֐�
{
    score -= 5000;//�X�R�A�����炷
    if (score < 0) {//�X�R�A��0�����̂Ƃ��̓X�R�A��0�Ƃ���(�X�R�A�̍Œ�l��0�Ƃ���)
        score = 0;
    }
    pldamagedTime = 0;//�v���C���[�̐F����莞�ԕς���(pldamagedTime�̌v���J�n)
}

void enemydraw(int damaged)//�G�̕`����s���֐�
{
    if (damaged == 0) {//�ʏ펞
        glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, pearlAmb);
        glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, pearlDiff);
        glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, pearlSpec);
        glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, pearlShin);
    }
    else {//�v���C���[�̒e������������
        glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, rubyAmb);//��u(����display�֐����Ă΂��܂�)�F��ς���
        glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, rubyDiff);
        glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, rubySpec);
        glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, rubyShin);
    }
    glPushMatrix();
    glTranslatef(enemy.c.x, enemy.c.y, enemy.c.z);
    glutSolidCube(enemy.c.size);
    glPopMatrix();
    glPushMatrix();
    glRotatef(-90.f, 1.f, 0.f, 0.f);
    glTranslatef(enemy.c.x - (float)(enemy.c.size / 3.9), -enemy.c.z, enemy.c.y + (float)(enemy.c.size / 2.0));
    glutSolidCone(enemy.c.size / 4.5, enemy.c.size / 2.5, 10, 10);
    glPopMatrix();
    glPushMatrix();
    glRotatef(-90.f, 1.f, 0.f, 0.f);
    glTranslatef(enemy.c.x + (float)(enemy.c.size / 3.9), -enemy.c.z, enemy.c.y + (float)(enemy.c.size / 2.0));
    glutSolidCone(enemy.c.size / 4.5, enemy.c.size / 2.5, 10, 10);
    glPopMatrix();
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, whiteplasticAmb);
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, whiteplasticDiff);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, whiteplasticSpec);
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, whiteplasticShin);
    glPushMatrix();
    glTranslatef(enemy.c.x - (float)(enemy.c.size / 3.8), enemy.c.y + (float)(enemy.c.size / 5.0), enemy.c.z + (float)(enemy.c.size / 2.0));
    glutSolidSphere(enemy.c.size / 6.0, 10, 10);
    glPopMatrix();
    glPushMatrix();
    glTranslatef(enemy.c.x + (float)(enemy.c.size / 3.8), enemy.c.y + (float)(enemy.c.size / 5.0), enemy.c.z + (float)(enemy.c.size / 2.0));
    glutSolidSphere(enemy.c.size / 6.0, 10, 10);
    glPopMatrix();
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, obsidianAmb);
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, obsidianDiff);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, obsidianSpec);
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, obsidianShin);
    glPushMatrix();
    glTranslatef(enemy.c.x - (float)(enemy.c.size / 3.8), enemy.c.y + (float)(enemy.c.size / 5.0), enemy.c.z + (float)(enemy.c.size / 1.6));
    glutSolidSphere(enemy.c.size / 13.0, 10, 10);
    glPopMatrix();
    glPushMatrix();
    glTranslatef(enemy.c.x + (float)(enemy.c.size / 3.8), enemy.c.y + (float)(enemy.c.size / 5.0), enemy.c.z + (float)(enemy.c.size / 1.6));
    glutSolidSphere(enemy.c.size / 13.0, 10, 10);
    glPopMatrix();
    glPushMatrix();
    glTranslatef(enemy.c.x, enemy.c.y, enemy.c.z + (float)(enemy.c.size / 2.0));
    glutSolidSphere(enemy.c.size / 15.0, 10, 10);
    glPopMatrix();
    glPushMatrix();
    glTranslatef(enemy.c.x - (float)(enemy.c.size * 3.0 / 16.0), enemy.c.y - (float)(enemy.c.size / 4.0), enemy.c.z + (float)(enemy.c.size / 2.0));
    for (int i = 0; i < 4; i++) {
        glutSolidCube(enemy.c.size / 8.0);
        glTranslatef((float)(enemy.c.size / 8.0), 0.f, 0.f);
    }
    glPopMatrix();
}

void reshape(int w, int h)
{
    glViewport(0, 0, w, h);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(30.0, double(w) / h, 0.1, 200.0);

    glMatrixMode(GL_MODELVIEW);
}

void timer(int value)
{
    glutPostRedisplay();
    glutTimerFunc(15, timer, value);    // 15�~���b��ɂ܂��^�C�}�[�֐����Ă�
}

//--------- ���̂̊e��ݒ�-----------//
void objectInit(void)
{
    srand((unsigned int)time(NULL));//rand()��������

    //�v���C���[�̏����ݒ�
    player.x = 0.f;
    player.y = 0.f;
    player.z = 1.f;
    player.size = 0.2;
    player.theta = 90;
    player.appear = 1;

    //�v���C���[�̒e�̏����ݒ�
    for (int i = 0; i < PLSHOT_NUM; i++) {
        plshot[i].size = 0.06;
        plshot[i].appear = 0;
    }

    //��Q���̏����ݒ�
    for (int i = 0; i < OBSTACLE_NUM; i++) {
        obstacle[i].hp = OBSTACLE_HP;
        obstacle[i].c.size = ((float)rand() / (float)RAND_MAX) * 1.0 + 0.3;
        obstacle[i].c.appear = 0;
    }

    //�G�̏����ݒ�
    enemy.hp = ENEMY_HP;
    enemy.c.x = 0.f;
    enemy.c.y = 0.f;
    enemy.c.z = -60.f;
    enemy.c.size = 1.0;
    enemy.c.theta = 0;
    enemy.c.appear = 0;

    //�G�̒e�̏����ݒ�
    for (int i = 0; i < ENSHOT_GRNUM; i++) {
        enshotgr[i].theta = 0;
        enshotgr[i].appear = 0;
    }
    for (int i = 0; i < ENSHOT_GRNUM * ENSHOT_NUM; i++) {
        enshot[i].size = 0.05;
        enshot[i].appear = 0;
    }
}

//--------- ���̑��e��ݒ�-----------//
void otherInit(void)
{
    glClearColor(0.f, 0.f, 0.f, 1.f);
    glClearDepth(1.f);
    glEnable(GL_DEPTH_TEST);

    //�����ݒ�(����g�U����ʂ̂�)
    glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmb);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiff);
    glLightfv(GL_LIGHT0, GL_SPECULAR, lightSpec);

    //�����ƃ��C�e�B���O�L����
    glEnable(GL_LIGHT0);
    glEnable(GL_LIGHTING);
}

//--------- �L�[�{�[�h�C�x���g�̏���------------//
void keyboard(unsigned char key, int x, int y) {
    printf("%c �����͂���܂���\n", key);
    switch (key) {
    case 'w':
        usingkeys[W_KEY] = 1;
        break;
    case 'a':
        usingkeys[A_KEY] = 1;
        break;
    case 's':
        usingkeys[S_KEY] = 1;
        break;
    case 'd':
        usingkeys[D_KEY] = 1;
        break;
    case ' ':
        usingkeys[SPACE_KEY] = 1;
        break;
    case 'q':
        if (gameEnd != 0) {//���ʉ�ʂ�'q'�L�[�������ꂽ�ꍇ�̓v���O�������I����
            exit(0);
        }
        break;
    default:
        break;
    }
}

void keyboardUp(unsigned char key, int x, int y) {
    switch (key) {
    case 'w':
        usingkeys[W_KEY] = 0;
        break;
    case 'a':
        usingkeys[A_KEY] = 0;
        break;
    case 's':
        usingkeys[S_KEY] = 0;
        break;
    case 'd':
        usingkeys[D_KEY] = 0;
        break;
    case ' ':
        usingkeys[SPACE_KEY] = 0;
        break;
    default:
        break;
    }
}

//---------- ���C���֐�-------------//
int main(int argc, char* argv[])
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE);
    glutInitWindowSize(640, 480);
    glutCreateWindow("3D Shooting");

    //�R�[���o�b�N�֐��o�^
    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard); // �L�[�{�[�h(�L�[�������ꂽ��)�p�̃R�[���o�b�N�֐��̐ݒ�
    glutKeyboardUpFunc(keyboardUp); // �L�[�{�[�h(�L�[�������ꂽ��)�p�̃R�[���o�b�N�֐��̐ݒ�
    glutReshapeFunc(reshape);
    glutTimerFunc(0, timer, 15);

    objectInit();//���̂̊e��ݒ�
    otherInit();//���̑��ݒ�

    glutMainLoop();

    return 0;
}