#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <time.h>
#include <gl/glut.h>
#define _USE_MATH_DEFINES
#include <math.h>

#define PLSHOT_NUM 10 //プレイヤーの弾数
#define PLSHOT_INTERVAL 5 //プレイヤーが弾を連続して撃てる間隔
#define PLDAMAGED_TIME 8 //プレイヤーがダメージを受けた際色が変わった状態となる時間の長さ
#define OBSTACLE_NUM 30 //障害物の数
#define OBSTACLE_HP 5 //障害物の初期HP
#define OBSTACLE_INTERVAL 30 //障害物が出てくる間隔
#define ENEMY_HP 30 //敵の初期HP
#define ENSHOT_INTERVAL 20 //敵が弾を撃つ間隔
#define ENSHOT_GRNUM 5 //敵の弾のグループ数
#define ENSHOT_NUM 9 //敵の弾の1グループあたりの数
#define STRSIZE 256 //strの最大の長さ

typedef struct {	//座標と大きさ、回転角、表示有無を保存する構造体
    float x;//x座標
    float y;//y座標
    float z;//z座標
    double size;//大きさ
    int theta;//回転角
    int appear;//この構造体に対応する物体が画面上に表示されているとき1となる変数(プレイヤーは無敵時間である間この変数が0となる)
} coordinate;

typedef struct {	//HPと座標、大きさ、回転角、表示有無を保存する構造体
    int hp;//HP
    coordinate c;
} HPandCoord;

coordinate player, plshot[PLSHOT_NUM], enshotgr[ENSHOT_GRNUM], enshot[ENSHOT_GRNUM * ENSHOT_NUM];//プレイヤー、プレイヤーの弾、敵の弾のグループ、敵の弾(実際の弾)の座標と大きさ、回転角、表示有無を保持する構造体
HPandCoord obstacle[OBSTACLE_NUM], enemy;//障害物と敵のHPと座標、大きさ、回転角、表示有無を保存する構造体

enum keys {
    W_KEY, A_KEY, S_KEY, D_KEY, SPACE_KEY
};

int usingkeys[5] = { 0 }; //各キーが押された状態であるとき1となる変数, インデックスはenum keysに対応

int score = 0;//ゲームのスコア
int plshotIdx = 0;//次に発射するプレイヤーの弾(配列の要素)のインデックスを格納する変数
int plshotTime = -1;//プレイヤーが弾を撃つ間隔を制御するための変数
int pldamagedTime = -1;//プレイヤーが障害物あるいは敵の弾と衝突した際一定時間色を変えるために用いる変数
int encoming = 0;//敵が出現した後所定のz座標に着くまでの間1となり、着いたら0となる変数
int enshotgrIdx = 0;////次に出てくる敵の弾のグループ(配列の要素)のインデックスを格納する変数
int enshotTime = -1;//敵が弾を撃つ間隔を制御するための変数
int obIdx = 0;//次に出てくる障害物(配列の要素)のインデックスを格納する変数
int obTime = 0;//障害物が登場する間隔を制御するための変数
int gameEnd = 0;//敵(障害物ではない)が倒されたら1とする変数
char str[STRSIZE];//画面に表示する文字列を格納する変数

void printstr(char* str);//画面に文字列を表示する関数
int collisionJudge(coordinate object1, coordinate object2);//衝突判定を行う関数, object1とobject2が接触(衝突)しているとき1, そうでないとき0を返す
void playerdamaged();//プレイヤーが障害物あるいは敵の弾と衝突した際の処理を行う関数
void enemydraw(int damaged);//敵の描画を行う関数

//------------ 各種材質パラメータ--------------//
//光源(環境光､拡散光､鏡面光､位置)
GLfloat lightAmb[] = { 0.f, 0.f, 0.f, 1.f };
GLfloat lightDiff[] = { 1.f, 1.f, 1.f, 1.f };
GLfloat lightSpec[] = { 1.f, 1.f, 1.f, 1.f };
GLfloat lightPos[] = { 1.f, 1.f, 1.f, 0.f };//平行光源

//材質 (環境光､拡散光､鏡面光､鏡面指数) のパラメータ（ルビー）
GLfloat rubyAmb[] = { 0.1745f, 0.01175f, 0.01175f, 1.f };
GLfloat rubyDiff[] = { 0.61424f, 0.04136f, 0.04136f, 1.f };
GLfloat rubySpec[] = { 0.727811f, 0.626959f, 0.626959f, 1.f };
GLfloat rubyShin = 76.8f;

//材質 (環境光､拡散光､鏡面光､鏡面指数) のパラメータ（ターコイズ）
GLfloat turquoiseAmb[] = { 0.1f, 0.18725f, 0.1745f, 1.f };
GLfloat turquoiseDiff[] = { 0.396f, 0.74151f, 0.69102f, 1.f };
GLfloat turquoiseSpec[] = { 0.297254f, 0.30829f, 0.306678f, 1.f };
GLfloat turquoiseShin = 12.8f;

//材質 (環境光､拡散光､鏡面光､鏡面指数) のパラメータ（パール）
GLfloat pearlAmb[] = { 0.25f, 0.20725f, 0.20725f, 1.f };
GLfloat pearlDiff[] = { 1.0f, 0.829f, 0.829f, 1.f };
GLfloat pearlSpec[] = { 0.296648f, 0.296648f, 0.296648f, 1.f };
GLfloat pearlShin = 11.264f;

//材質 (環境光､拡散光､鏡面光､鏡面指数) のパラメータ（白いプラスチック）
GLfloat whiteplasticAmb[] = { 0.f, 0.f, 0.f, 1.f };
GLfloat whiteplasticDiff[] = { 0.55f, 0.55f, 0.55f, 1.f };
GLfloat whiteplasticSpec[] = { 0.70f, 0.70f, 0.70f, 1.f };
GLfloat whiteplasticShin = 32.f;

//材質 (環境光､拡散光､鏡面光､鏡面指数) のパラメータ（黒曜石）
GLfloat obsidianAmb[] = { 0.05375f, 0.05f, 0.06625f, 1.f };
GLfloat obsidianDiff[] = { 0.18275f, 0.17f, 0.22525f, 1.f };
GLfloat obsidianSpec[] = { 0.332741f, 0.328634f, 0.346435f, 1.f };
GLfloat obsidianShin = 38.4f;

//材質 (環境光､拡散光､鏡面光､鏡面指数) のパラメータ（金）
GLfloat goldAmb[] = { 0.24725f, 0.1995f, 0.0745f, 1.f };
GLfloat goldDiff[] = { 0.75164f, 0.60648f, 0.22648f, 1.f };
GLfloat goldSpec[] = { 0.628281f, 0.555802f, 0.366065f, 1.f };
GLfloat goldShin = 51.2f;

//--------- 各種コールバック関数-------//
void display(void)
{
    static int angle = 0;

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    gluLookAt(0.0, 0.5, 4.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);

    //光源の位置設定
    glLightfv(GL_LIGHT0, GL_POSITION, lightPos);

    //キー入力反映
    if (player.y < 0.75f) {//まだ上に行ける場合
        if (usingkeys[W_KEY] == 1) { //'w'キーが入力されている時
            player.y += 0.02f;//上に平行移動
        }
    }
    if (player.x > -0.82f) {//まだ左に行ける場合
        if (usingkeys[A_KEY] == 1) { //'a'キーが入力されている時
            player.x -= 0.02f;//左に平行移動
        }
    }
    if (player.y > -0.32f) {//まだ下に行ける場合
        if (usingkeys[S_KEY] == 1) { //'s'キーが入力されている時
            player.y -= 0.02f;//下に平行移動
        }
    }
    if (player.x < 0.82f) {//まだ右に行ける場合
        if (usingkeys[D_KEY] == 1) { //'d'キーが入力されている時
            player.x += 0.02f;//右に平行移動
        }
    }
    if (plshotTime == -1 && usingkeys[SPACE_KEY] == 1) {//プレイヤーが弾を打てる状態で、かつスペースキーが押されているとき
        int plshotok = 1;//まだ描画されていないplshot[]の要素が1つもなければ0であり、1つでもあれば1であるとする変数
        if (plshot[plshotIdx].appear == 1) {//もしplshot[plshotIdx]が既に描画中であれば、まだ描画されていないplshot[]の要素を探す
            plshotok = 0;//まだ描画されていないplshot[]の要素が1つもなかったとき用に0としておく
            for (int i = 0; i < PLSHOT_NUM; i++) {
                if (plshot[i].appear == 0) {//まだ描画されていないplshot[]の要素が見つかった場合
                    plshotIdx = i;//plshotIdxを見つけた要素のインデックスの値にする
                    plshotok = 1;//まだ描画されていないplshot[]の要素が見つかったのでplshotを1に戻す
                }
            }
        }
        if (plshotok == 1) {//まだ描画されていないplshot[]の要素が1つでもあればその要素(plshot[plshotIdx])の描画を開始する
            plshotTime++;//plshotTimeの計測開始
            plshot[plshotIdx].x = player.x;//プレイヤーがいる位置をこの弾の初期位置とする
            plshot[plshotIdx].y = player.y;
            plshot[plshotIdx].z = player.z;
            plshot[plshotIdx].appear = 1;//plshotIdxに対応するプレイヤーの弾を出現させる
        }
        plshotIdx++;//plshotIdxを更新する
        if (plshotIdx >= PLSHOT_NUM) {//plshotIdxがplshotのインデックス数を超えたとき
            plshotIdx = 0;//plshotIdxを初期化する
        }
    }

    //障害物の描画
    if (obTime >= 0) {//obTimeが0以上のときobTimeの計測を行う
        obTime++;
        if (obTime > OBSTACLE_INTERVAL) {//obTimeの計測が始まってから既定の間隔が経過したとき
            obTime = 0;
            obstacle[obIdx].c.x = (float)(((((float)rand() / (float)RAND_MAX) * 2.0) - 1.0) * 0.82);//初期位置はプレイヤーの移動可能範囲内でランダムな位置とする
            obstacle[obIdx].c.y = (float)((((float)rand() / (float)RAND_MAX) * (0.75 + 0.32)) - 0.32);
            obstacle[obIdx].c.z = -30.f;
            obstacle[obIdx].c.appear = 1;//obIdxに対応する障害物を出現させる
            obIdx++;//obIdxを更新する
            if (obIdx >= OBSTACLE_NUM) {//obIdxがobstacleのインデックス数を超えたとき
                obTime = -1;//obTimeの計測を停止する
            }
        }
    }
    int obappear = 0;//描画される障害物が1つでもあるとき1になる変数
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
                int obcollision = 0;//この障害物がプレイヤーの弾と衝突しているとき1とする変数
                for (int j = 0; j < PLSHOT_NUM; j++) {
                    if (plshot[j].appear == 1) {//この障害物と衝突判定を行うのは描画されているプレイヤーの弾のみとする
                        if (collisionJudge(obstacle[i].c, plshot[j]) == 1) {//この障害物がプレイヤーの弾と衝突しているとき
                            obcollision = 1;
                            plshot[j].appear = 0;//衝突しているプレイヤーの弾を消す
                            obstacle[i].hp--;//この障害物のHPを1減らす
                            glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, rubyAmb);//一瞬(つぎにこの関数が呼ばれるまで)色を変える
                            glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, rubyDiff);
                            glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, rubySpec);
                            glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, rubyShin);
                            break;//同時(この関数の1度の実行中)に複数のプレイヤーの弾が当たることはないようにする
                        }
                    }
                }
                if (obcollision == 0) {
                    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, turquoiseAmb);
                    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, turquoiseDiff);
                    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, turquoiseSpec);
                    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, turquoiseShin);
                }
                glutSolidCube(obstacle[i].c.size);//描画
                glPopMatrix();
                if (obstacle[i].hp <= 0) {//この障害物のHPがなくなったとき
                    score += 10000;//スコアを増やす
                    obstacle[i].c.appear = 0;//この障害物を消す
                }
            }
        }
    }
    if (obTime == -1 && obappear == 0 && encoming == 0 && enemy.c.appear == 0 && gameEnd == 0) {//全ての障害物の出現と消滅が終わっており、まだ敵が現れていない場合
        encoming = 1;//敵の接近を開始する
    }

    //敵の描画
    if (encoming == 1) {//敵を所定のz座標まで移動させる
        enemy.c.z += 0.1f;
        enemydraw(0);//描画
        if (enemy.c.z >= -10.f) {//敵が所定のz座標まで着いたら敵が出現した状態であるとする
            encoming = 0;
            enemy.c.appear = 1;
            enshotTime++;//enshotTimeの計測開始
        }
    }
    if (enemy.c.appear == 1) {
        enemy.c.y = 0.3f * (float)sin((((double)enemy.c.theta * 2.0) / 180.0) * M_PI);
        enemy.c.x = 0.8f * (float)sin((((double)enemy.c.theta / 180.0) * M_PI));
        int encollision = 0;//敵がプレイヤーの弾と衝突しているとき1とする変数
        for (int i = 0; i < PLSHOT_NUM; i++) {
            if (plshot[i].appear == 1) {//敵と衝突判定を行うのは描画されているプレイヤーの弾のみとする
                if (collisionJudge(enemy.c, plshot[i]) == 1) {//敵がプレイヤーの弾と衝突しているとき
                    encollision = 1;
                    plshot[i].appear = 0;//衝突しているプレイヤーの弾を消す
                    enemy.hp--;//敵のHPを1減らす
                    break;//同時(この関数の1度の実行中)に複数のプレイヤーの弾が当たることはないようにする
                }
            }
        }
        enemydraw(encollision);//描画
        if (enemy.hp <= 0) {//敵のHPがなくなったとき
            score += 100000;//スコアを増やす
            enemy.c.appear = 0;//敵を消す
            gameEnd = 1;//結果画面の表示に移る
        }
        enemy.c.theta = (enemy.c.theta + 4) % 360;
    }

    //敵の弾の描画
    if (enshotTime >= 0) {//enshotTimeが0以上のときenshotTimeの計測を行う
        enshotTime++;
        if (enshotTime > ENSHOT_INTERVAL && enshotgr[enshotgrIdx].appear == 0) {//enshotTimeの計測が始まってから既定の間隔が経過しており、かつ出現(描画)されていない敵の弾のグループがあるとき
            enshotTime = 0;
            enshotgr[enshotgrIdx].x = enemy.c.x;//敵がいる位置をこの敵の弾のグループの初期位置とする
            enshotgr[enshotgrIdx].y = enemy.c.y;
            enshotgr[enshotgrIdx].z = enemy.c.z;
            enshotgr[enshotgrIdx].appear = 1;//enshotgrIdxに対応する敵の弾のグループを出現させる
            for (int i = enshotgrIdx * ENSHOT_NUM; i < (enshotgrIdx + 1) * ENSHOT_NUM; i++) {//このグループに属する敵の弾を出現させる
                enshot[i].appear = 1;
            }
            enshotgrIdx++;//enshotgrIdxを更新する
            if (enshotgrIdx >= ENSHOT_GRNUM) {//grIdxがenshotgrのインデックス数を超えたとき
                enshotgrIdx = 0;//enshotgrIdxを初期化する
            }
        }
    }
    for (int i = 0; i < ENSHOT_GRNUM; i++) {
        if (enshotgr[i].appear == 1) {
            enshotgr[i].z += 0.1f;
            if (enshotgr[i].z >= 1.4f || enemy.c.appear == 0) {//描画範囲外に出たとき以外に、敵が倒されたときも敵の弾のグループ及び敵の弾を消す処理を行う
                enshotgr[i].appear = 0;
                for (int j = i * ENSHOT_NUM; j < (i + 1) * ENSHOT_NUM; j++) {//このグループに属する敵の弾も消す
                    enshot[j].appear = 0;
                }
            }
            else {
                enshotgr[i].theta = (enshotgr[i].theta + 5) % 360;
                for (int j = i * ENSHOT_NUM; j < (i + 1) * ENSHOT_NUM; j++) {//このグループに属する敵の弾の描画
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
                        glutSolidSphere(enshot[j].size, 10, 10);//描画
                        glPopMatrix();
                    }
                }
            }
        }
    }

    //プレイヤーの弾の描画
    if (plshotTime >= 0) {//plshotTimeが0以上のときplshotTimeの計測を行う
        plshotTime++;
        if (plshotTime > PLSHOT_INTERVAL) {//plshotTimeの計測が始まってから既定の間隔が経過したとき
            plshotTime = -1;//plshotTimeを初期化する
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
                glutSolidSphere(plshot[i].size, 10, 10);//描画
                glPopMatrix();
            }
        }
    }

    //プレイヤーの描画
    glPushMatrix();
    glRotatef((float)player.theta, 0.f, 1.f, 0.f);
    glTranslatef(-player.z, player.y, player.x);
    for (int i = 0; i < OBSTACLE_NUM; i++) {
        if (obstacle[i].c.appear == 1) {//プレイヤーと衝突判定を行う障害物は描画されているもののみとする
            if (collisionJudge(player, obstacle[i].c) == 1) {//プレイヤーが障害物と衝突しているとき
                obstacle[i].c.appear = 0;//衝突している障害物を消す
                playerdamaged();
                break;//同時(この関数の1度の実行中)に複数の障害物が当たることはないようにする
            }
        }
    }
    for (int i = 0; i < ENSHOT_GRNUM * ENSHOT_NUM; i++) {
        if (enshot[i].appear == 1) {//プレイヤーと衝突判定を行う敵の弾は描画されているもののみとする
            if (collisionJudge(player, enshot[i]) == 1) {//プレイヤーが敵の弾と衝突しているとき
                enshot[i].appear = 0;//衝突している敵の弾を消す
                playerdamaged();
                break;//同時(この関数の1度の実行中)に複数の敵の弾が当たることはないようにする
            }
        }
    }
    if (pldamagedTime == -1) {
        glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, goldAmb);
        glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, goldDiff);
        glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, goldSpec);
        glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, goldShin);
    }
    else{//pldamagedTimeが0以上のときplshotTimeの計測を行う
        pldamagedTime++;
        glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, rubyAmb);//プレイヤーの色を通常時と異なる色にする
        glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, rubyDiff);
        glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, rubySpec);
        glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, rubyShin);
        if (pldamagedTime > PLDAMAGED_TIME) {//pldamagedTimeの計測が始まってから既定の時間が経過したとき
            pldamagedTime = -1;//pldamagedTimeを初期化する
        }
    }
    glutSolidTeapot(player.size);//描画
    glPopMatrix();

    //画面上への文字列表示
    glPushMatrix();
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, pearlAmb);
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, pearlDiff);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, pearlSpec);
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, pearlShin);
    glNormal3f(0.f, 0.f, 1.1f);
    if (gameEnd == 0) {//通常画面の文字列表示
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
    else {//結果画面の文字列表示
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

void printstr(char* str)//画面に文字列を表示する関数
{
    for (int i = 0; i < (signed)strlen(str); i++) {
        char ch = str[i];
        glutBitmapCharacter(GLUT_BITMAP_9_BY_15, ch);
    }
}

int collisionJudge(coordinate object1, coordinate object2)//衝突判定を行う関数, object1とobject2が接触(衝突)しているとき1, そうでないとき0を返す
{
    double distance;//2つの物体の座標間の距離(中心間の距離)を格納する変数
    distance = sqrt((((double)object1.x - (double)object2.x) * ((double)object1.x - (double)object2.x))
        + (((double)object1.y - (double)object2.y) * ((double)object1.y - (double)object2.y))
        + (((double)object1.z - (double)object2.z) * ((double)object1.z - (double)object2.z)));//距離の計算を実行する
    if (distance <= (object1.size + object2.size) / 2.0) {//2つの物体を球と見て、distanceが半径の和以下なら接触(衝突)しているとする
        return 1;
    }

    return 0;
}

void playerdamaged()//プレイヤーが障害物あるいは敵の弾と衝突した際の処理を行う関数
{
    score -= 5000;//スコアを減らす
    if (score < 0) {//スコアが0未満のときはスコアを0とする(スコアの最低値を0とする)
        score = 0;
    }
    pldamagedTime = 0;//プレイヤーの色を一定時間変える(pldamagedTimeの計測開始)
}

void enemydraw(int damaged)//敵の描画を行う関数
{
    if (damaged == 0) {//通常時
        glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, pearlAmb);
        glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, pearlDiff);
        glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, pearlSpec);
        glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, pearlShin);
    }
    else {//プレイヤーの弾が当たった時
        glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, rubyAmb);//一瞬(つぎにdisplay関数が呼ばれるまで)色を変える
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
    glutTimerFunc(15, timer, value);    // 15ミリ秒後にまたタイマー関数を呼ぶ
}

//--------- 物体の各種設定-----------//
void objectInit(void)
{
    srand((unsigned int)time(NULL));//rand()を初期化

    //プレイヤーの初期設定
    player.x = 0.f;
    player.y = 0.f;
    player.z = 1.f;
    player.size = 0.2;
    player.theta = 90;
    player.appear = 1;

    //プレイヤーの弾の初期設定
    for (int i = 0; i < PLSHOT_NUM; i++) {
        plshot[i].size = 0.06;
        plshot[i].appear = 0;
    }

    //障害物の初期設定
    for (int i = 0; i < OBSTACLE_NUM; i++) {
        obstacle[i].hp = OBSTACLE_HP;
        obstacle[i].c.size = ((float)rand() / (float)RAND_MAX) * 1.0 + 0.3;
        obstacle[i].c.appear = 0;
    }

    //敵の初期設定
    enemy.hp = ENEMY_HP;
    enemy.c.x = 0.f;
    enemy.c.y = 0.f;
    enemy.c.z = -60.f;
    enemy.c.size = 1.0;
    enemy.c.theta = 0;
    enemy.c.appear = 0;

    //敵の弾の初期設定
    for (int i = 0; i < ENSHOT_GRNUM; i++) {
        enshotgr[i].theta = 0;
        enshotgr[i].appear = 0;
    }
    for (int i = 0; i < ENSHOT_GRNUM * ENSHOT_NUM; i++) {
        enshot[i].size = 0.05;
        enshot[i].appear = 0;
    }
}

//--------- その他各種設定-----------//
void otherInit(void)
{
    glClearColor(0.f, 0.f, 0.f, 1.f);
    glClearDepth(1.f);
    glEnable(GL_DEPTH_TEST);

    //光源設定(環境､拡散､鏡面のみ)
    glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmb);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiff);
    glLightfv(GL_LIGHT0, GL_SPECULAR, lightSpec);

    //光源とライティング有効化
    glEnable(GL_LIGHT0);
    glEnable(GL_LIGHTING);
}

//--------- キーボードイベントの処理------------//
void keyboard(unsigned char key, int x, int y) {
    printf("%c が入力されました\n", key);
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
        if (gameEnd != 0) {//結果画面で'q'キーが押された場合はプログラムを終える
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

//---------- メイン関数-------------//
int main(int argc, char* argv[])
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE);
    glutInitWindowSize(640, 480);
    glutCreateWindow("3D Shooting");

    //コールバック関数登録
    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard); // キーボード(キーが押された時)用のコールバック関数の設定
    glutKeyboardUpFunc(keyboardUp); // キーボード(キーが離された時)用のコールバック関数の設定
    glutReshapeFunc(reshape);
    glutTimerFunc(0, timer, 15);

    objectInit();//物体の各種設定
    otherInit();//その他設定

    glutMainLoop();

    return 0;
}