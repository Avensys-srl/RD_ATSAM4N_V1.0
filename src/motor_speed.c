/*
 * motor_speed.cpp
 *
  *  Descrizione: tutte le funzioni legate al pilotaggio dei motori
 */ 

#include "asf.h"

#include "eeprom_map.h"
#include "Protocol_Ser1.h"
#include "data.h"
#include "alarm.h"
#include <math.h>

#define byte	Byte
#define word	uint16_t

#ifdef VIEW_DEBUG_CODE_NO_KTS
#define VIEW_MOTORSPEED_DBG  // se dichairato visualizza lo stato della velocita' dei motori
#ifdef VIEW_MOTORSPEED_DBG //''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''' x_dbg  
char cnt_view_dbg = 0;
#endif

#endif 

#define COMESSA_2388

// stato del Rele' e delle tensione dell'elettronica
#define POWER_SAVE     0
#define POWER_ON      'O'

//--------------------------------------------------------------------
int  SetpointPa           = 100;   // si parte con 100 pascal

//--------------------------------------------------------------------
#define SOGLIA_TF_DEFROST        -50     // -5.0 C
#define SOGLIA_CICLO_DEFROST    1200     // 1200 sec. = 20 min.

#define MINSPEED            64  
#define MINSPEED_STARTUP    85           // velocita'  al 33%.

#define TIMER_MAX          250


//--------------------------------------------------
unsigned short GetMaxAirFlow();
extern Byte AccessoryPresent_EepList(int nAccEep);
extern void TAG_DigitAccessoryOff(int nAcc_I2C);
extern int update_data_eeprom(unsigned short start_addr, unsigned short end_addr, Byte *buff);
extern void probeQAirActive();
extern void PowerMode(Byte sts);

//''''''''''''''''''''''''''''''''''''' x_dbg
int cnt_dbg = 0;
//'''''''''''''''''''''''''''''''''''''
// contatori
int count_sec_defrost         = 0;
int count_post_vent           = 0;
int count_switch_off          = 0;
int count_switch_on           = 0;
int count_Boost_on            = 0;
int cnt_restart_motors        = 0;
int cnt_correct_minSpeedMotors_EHD_ON  = 0;
byte num_tentativi_ripartenza = 0;

int persist_temp_defrost_on = 0; 

// velocita' motori
Byte temp_speed_motorsF = 0;   // variabile temporanea, per configurare la velocita' dei motori F
Byte temp_speed_motorsR = 0;   // variabile temporanea, per configurare la velocita' dei motori R
Byte minSpeedMotors_HeaterON = 51;
Byte CO2T_speed;

// sbilanciamento motori in defrost
float unbalance_deFrost = 1.0;
float conta_step=0;

//--------------------------------
int acc_measure_pressure    = 0;
int sample_measure_pressure = 0;
int acc_measure_airflow     = 0;
int sample_measure_airflow  = 0;

#define NUM_AVERAGE_RPM  4
unsigned short rpm_r_preview[NUM_AVERAGE_RPM];

//----------------------------------------------------------------------------
//                   Tabelle per il calcolo del CAP/CAF
const int KMax	= 100;
const int KMin	= 40;
//const int KVolts_Length	= 61;
//const int KRPM_Length	= 61;

extern	SDATA  sData;
extern Bool output_on;
extern unsigned long countHoursFunz;
extern short cnt_no_Pressure_for_Electrical_Heater;

extern short Pressure_CAPS;
extern short Pressure_CAPR;

extern short Airflow_CAPS;
extern short Airflow_CAPR;

extern volatile uint32_t Clean_Event_Counter;

extern bool Clean_Event_Flag1;
extern bool Clean_Event_Flag2;

extern Byte Index_CO2_1;
extern Byte Index_CO2_2;

extern volatile uint32_t CO2_1_Average_Counter;
extern volatile uint32_t CO2_2_Average_Counter;

extern volatile uint32_t Clean_Event_Counter_2_hours;

//-----------  stato rele' unita ---------------------
extern Byte PowerMotors;

extern volatile uint32_t pir_counter;
bool pir_counter_flag = false;
bool Power_On_Already_Activated = false;
char Buff_Test[100];
extern bool Pir_Startup;

uint32_t volatile PIR_Delay_Counter = 0;

//---------------- mappatura motore Quantum (0025,..,0055) e serie 8 -----------------------
#ifdef COMESSA_2388
// Taglia CLRC 068
enum {  KVolts_Length = 61 };
const double gKVolts[ KVolts_Length ][ 4 ] =
{
  {-000.0000024316401,   000.0020881770724,  -001.5549455947575,   816.6367872746436},
  {-000.0000024969623,   000.0021334640452,  -001.5711833274514,   813.6171207216547},
  {-000.0000025633694,   000.0021788689387,  -001.5874155341553,   810.5720047481666},
  {-000.0000026307577,   000.0022242804341,  -001.6036178031339,   807.4996068184849},
  {-000.0000026990014,   000.0022695715532,  -001.6197630224765,   804.3979681497978},
  {-000.0000027679492,   000.0023145978281,  -001.6358211242487,   801.2649952882828},
  {-000.0000028374211,   000.0023591952662,  -001.6517588059888,   798.0984512265723},
  {-000.0000029072039,   000.0024031780865,  -001.6675392278407,   794.8959460633945},
  {-000.0000029770464,   000.0024463362046,  -001.6831216835609,   791.6549272132241},
  {-000.0000030466547,   000.0024884324382,  -001.6984612436078,   788.3726691826920},
  {-000.0000031156855,   000.0025291994040,  -001.7135083685145,   785.0462629415370},
  {-000.0000032875524,   000.0026114935888,  -001.7220412082877,   776.4532772524474},
  {-000.0000034702927,   000.0026960065161,  -001.7300816541974,   767.7792534101791},
  {-000.0000036650010,   000.0027829846818,  -001.7376305376537,   759.0241674068983},
  {-000.0000038729375,   000.0028727257847,  -001.7446938027957,   750.1882032361620},
  {-000.0000040955581,   000.0029655889324,  -001.7512834768954,   741.2717739625102},
  {-000.0000043345516,   000.0030620068545,  -001.7574187961453,   732.2755442875909},
  {-000.0000045918847,   000.0031625005280,  -001.7631275117808,   723.2004546653769},
  {-000.0000048698566,   000.0032676967053,  -001.7684474053605,   714.0477469951753},
  {-000.0000051711651,   000.0033783489369,  -001.7734280462885,   704.8189918820298},
  {-000.0000054989878,   000.0034953628029,  -001.7781328292048,   695.5161173912175},
  {-000.0000059851763,   000.0038623141834,  -001.8761081517870,   693.0380233038367},
  {-000.0000064692825,   000.0042245533006,  -001.9722918595006,   690.2675174987068},
  {-000.0000069430901,   000.0045776857706,  -002.0660627558584,   687.1781968858837},
  {-000.0000073966826,   000.0049167453686,  -002.1567714315826,   683.7441183071124},
  {-000.0000078183799,   000.0052362690041,  -002.2437627251337,   679.9407509416267},
  {-000.0000081947791,   000.0055304388403,  -002.3264070358170,   675.7461413316736},
  {-000.0000085109462,   000.0057933086228,  -002.4041413118308,   671.1422618864440},
  {-000.0000087508084,   000.0060191287173,  -002.4765195914037,   666.1164704161747},
  {-000.0000088977934,   000.0062027774777,  -002.5432714590420,   660.6629512266003},
  {-000.0000089357509,   000.0063402940304,  -002.6043646515415,   654.7839414779825},
  {-000.0000103108447,   000.0069990814854,  -002.7072122319699,   649.8260572004744},
  {-000.0000118855317,   000.0077244778846,  -002.8161659225030,   644.8937676516506},
  {-000.0000136873580,   000.0085214942953,  -002.9312161393655,   639.9642209973511},
  {-000.0000157459620,   000.0093943313406,  -003.0521411253001,   635.0052471654539},
  {-000.0000180919879,   000.0103455837082,  -003.1784021648176,   629.9721561466168},
  {-000.0000207549874,   000.0113750082929,  -003.3089942902805,   624.8034607384865},
  {-000.0000237596590,   000.0124776379105,  -003.4422342647712,   619.4151834529123},
  {-000.0000271193969,   000.0136409181761,  -003.5754607723090,   613.6933310609518},
  {-000.0000308255321,   000.0148403962986,  -003.7046131796431,   607.4840658430486},
  {-000.0000348297494,   000.0160332864736,  -003.8236457353229,   600.5811274774127},
  {-000.0000371747291,   000.0168197182304,  -003.9089489433501,   591.1944928190914},
  {-000.0000396894970,   000.0176425696105,  -003.9954557695385,   581.6522450416203},
  {-000.0000423816619,   000.0185014864124,  -004.0828723594634,   571.9345182806392},
  {-000.0000452576026,   000.0193955901290,  -004.1708427966110,   562.0190658760020},
  {-000.0000483220424,   000.0203234001765,  -004.2589406277193,   551.8809065261968},
  {-000.0000515776167,   000.0212827617936,  -004.3466580713749,   541.4918458882516},
  {-000.0000550244778,   000.0222707784753,  -004.4333906636161,   530.8197994526936},
  {-000.0000586599854,   000.0232837348986,  -004.5184130599585,   519.8278008997793},
  {-000.0000624785000,   000.0243169663013,  -004.6008382053699,   508.4725201854723},
  {-000.0000664711776,   000.0253645673553,  -004.6795462308584,   496.7020332936038},
  {-000.0000852059995,   000.0303666401223,  -005.1103287190641,   490.0834732443518},
  {-000.0001099634686,   000.0365673151413,  -005.6132425350111,   484.0343188847173},
  {-000.0001428609071,   000.0442923730746,  -006.2036347318848,   478.6207324013510},
  {-000.0001868221878,   000.0539785073265,  -006.9021513977254,   473.9350916543278},
  {-000.0002460579625,   000.0662610365520,  -007.7396550788053,   470.1416395499682},
  {-000.0003272736968,   000.0822013164179,  -008.7686469098043,   467.5777313691375},
  {-000.0004430735313,   000.1038688384018,  -010.0879497273467,   466.9363045315127},
  {-000.0006214827609,   000.1357155751862,  -011.8886924692996,   469.4734129345706},
  {-000.0009296412682,   000.1872293977602,  -014.5088336097587,   476.7431142225039},
  {-000.0015092788578,   000.2744449087509,  -018.3235203821068,   487.4087554864439},
 };
#else
enum {  KVolts_Length = 61 };
const double gKVolts[ KVolts_Length ][ 4 ] =
{
  {-0.0000015563999,   0.0001644005834,  -0.4892715767313,   658.5462286336630},
  {-0.0000010562992,  -0.0004437604426,  -0.2743198197851,   633.4947588458350},
  {-0.0000005395383,  -0.0010648151774,  -0.0573211649738,   608.4935913041866},
  {-0.0000000060253,  -0.0016985589154,   0.1615743352364,   583.5629549059206},
  { 0.0000005443118,  -0.0023447635560,   0.3822104960278,   558.7233181839128},
  { 0.0000011115238,  -0.0030031770399,   0.6044250578381,   533.9953598728490},
  { 0.0000016956394,  -0.0036735228066,   0.8280497570315,   509.3999382312358},
  { 0.0000022966637,  -0.0043554992744,   1.0529104094739,   484.9580591977175},
  { 0.0000029145777,  -0.0050487793422,   1.2788270067075,   460.6908434729684},
  { 0.0000035493362,  -0.0057530099139,   1.5056138242762,   436.6194926312348},
  { 0.0000042008673,  -0.0064678114449,   1.7330795415923,   412.7652543784504},
  { 0.0000032272758,  -0.0053327303840,   1.3384595806822,   437.8527436827967},
  { 0.0000022131066,  -0.0041812229377,   0.9491903311656,   461.3171680739961},
  { 0.0000011585969,  -0.0030153803517,   0.5662521213489,   483.0771973094340},
  { 0.0000000642461,  -0.0018375821524,   0.1906896228759,   503.0503880587472},
  {-0.0000010691299,  -0.0006505370229,  -0.1763814256144,   521.1531635440725},
  {-0.0000022403371,   0.0005426728080,  -0.5337676622625,   537.3009065691336},
  {-0.0000034477332,   0.0017385443186,  -0.8801920850626,   551.4082048159798},
  {-0.0000046891492,   0.0029331046732,  -1.2142890634529,   563.3892937870050},
  {-0.0000059618046,   0.0041218620495,  -1.5346010032533,   573.1587489466050},
  {-0.0000072622162,   0.0052997586106,  -1.8395774401644,   580.6324836997939},
  {-0.0000072216777,   0.0052019406744,  -1.7923711674136,   565.3236040948005},
  {-0.0000071624368,   0.0050879333572,  -1.7410124994214,   549.7238752055682},
  {-0.0000070830397,   0.0049568844910,  -1.6853818013474,   533.8319199741580},
  {-0.0000069819788,   0.0048079402646,  -1.6253672890370,   517.6473653963731},
  {-0.0000068576962,   0.0046402509809,  -1.5608665844666,   501.1709418350138},
  {-0.0000067085893,   0.0044529775960,  -1.4917883891425,   484.4045846532175},
  {-0.0000065330174,   0.0042452990847,  -1.4180542726177,   467.3515372399358},
  {-0.0000063293097,   0.0040164206701,  -1.3396005708120,   450.0164543113331},
  {-0.0000060957747,   0.0037655829469,  -1.2563803859351,   432.4055041695000},
  {-0.0000058307115,   0.0034920719167,  -1.1683656765344,   414.5264683869658},
  {-0.0000064576173,   0.0040389581408,  -1.3117733460701,   414.4692772221414},
  {-0.0000071311671,   0.0046136673838,  -1.4580269198253,   414.2769305751934},
  {-0.0000078545389,   0.0052171783390,  -1.6069924011982,   413.9256254641818},
  {-0.0000086309975,   0.0058503652880,  -1.7584860592121,   413.3881263267162},
  {-0.0000094638502,   0.0065139519812,  -1.9122639941086,   412.6332641517816},
  {-0.0000103563844,   0.0072084540503,  -2.0680098503181,   411.6253750708806},
  {-0.0000113117835,   0.0079341077197,  -2.2253204391999,   410.3236776749260},
  {-0.0000123330151,   0.0086907823278,  -2.3836890427645,   408.6815916319604},
  {-0.0000134226854,   0.0094778739573,  -2.5424862009413,   406.6460051170631},
  {-0.0000145828527,   0.0102941773750,  -2.7009378498161,   404.1565056556400},
  {-0.0000133029140,   0.0091795705652,  -2.4244722009672,   377.9465986531855},
  {-0.0000118168067,   0.0079552619805,  -2.1335158379516,   351.3906585330855},
  {-0.0000101005524,   0.0066131380307,  -1.8276699577615,   324.5106677126344},
  {-0.0000081276134,   0.0051446559614,  -1.5065771429459,   297.3330837371665},
  {-0.0000058686520,   0.0035408580265,  -1.1699331262925,   269.8893457498181},
  {-0.0000032912809,   0.0017924000890,  -0.8175011207886,   242.2164502709137},
  {-0.0000003598141,  -0.0001103993368,  -0.4491294626398,   214.3576158556604},
  { 0.0000029649702,  -0.0021774810524,  -0.0647735873559,   186.3630632229220},
  { 0.0000067260310,  -0.0044189259733,   0.3354762650352,   158.2909467399914},
  { 0.0000109701966,  -0.0068447773568,   0.7513597004639,   130.2084853315442},
  { 0.0000105413310,  -0.0063556553040,   0.6309541373848,   128.0130347500416},
  { 0.0000099327099,  -0.0057747962068,   0.5003458055721,   125.9634438228579},
  { 0.0000090974038,  -0.0050854657021,   0.3582818373108,   124.0732001956456},
  { 0.0000079756303,  -0.0042673985596,   0.2033137643005,   122.3570602864017},
  { 0.0000064908319,  -0.0032959390945,   0.0337628029135,   120.8310325461131},
  { 0.0000045444187,  -0.0021409487617,  -0.1523205261051,   119.5122513743889},
  { 0.0000020086765,  -0.0007654159418,  -0.3572023882422,   118.4186611692086},
  {-0.0000012828504,   0.0008763117105,  -0.5835160648776,   117.5683764413109},
  {-0.0000055484618,   0.0028407644147,  -0.8343133226714,   116.9784933790901},
  {-0.0000110761986,   0.0051981504229,  -1.1131085388064,   116.6629732395725},
};
#endif

enum {  KRPM_Length = 61 };
const float gKRPM[ KRPM_Length ][ 4 ] =
{ 
  { 0.000003604,  -000.031962167,   00093.361754410,  -0089410.266971680},
  { 0.000003052,  -000.027049299,   00078.819076156,  -0075111.289841854},
  { 0.000002494,  -000.022122824,   00064.357186836,  -0061007.510032809},
  { 0.000001932,  -000.017202560,   00050.032672112,  -0047151.234247493},
  { 0.000001368,  -000.012311128,   00035.909034552,  -0033600.311715587},
  { 0.000000807,  -000.007474157,   00022.057088910,  -0020418.366735487},
  { 0.000000250,  -000.002720394,   00008.555104951,  -0007674.787749314},
  {-0.000000298,   000.001918300,  -00004.511395536,   0004555.606204007},
  {-0.000000832,   000.006407120,  -00017.050287134,   0016193.303613615},
  {-0.000001348,   000.010708685,  -00028.964031786,   0027155.319759869},
  {-0.000001842,   000.014783650,  -00040.151494435,   0037356.956831170},
  {-0.000001411,   000.011051941,  -00029.461244797,   0027205.066872126},
  {-0.000000907,   000.006784160,  -00017.506923385,   0016095.155747689},
  {-0.000000325,   000.001959425,  -00004.272806035,   0004044.722098660},
  { 0.000000341,  -000.003440666,   00010.248219234,  -0008920.657438441},
  { 0.000001093,  -000.009432210,   00026.055425070,  -0022768.354857677},
  { 0.000001938,  -000.016029234,   00043.141120958,  -0037459.764360248},
  { 0.000002878,  -000.023243914,   00061.491479983,  -0052951.285590149},
  { 0.000003918,  -000.031086732,   00081.087137823,  -0069195.067548515},
  { 0.000005062,  -000.039566471,   00101.903362771,  -0086139.356735967},
  { 0.000006314,  -000.048689951,   00123.909572916,  -0103728.287320799},
  { 0.000006633,  -000.050542061,   00127.105660047,  -0105172.547502706},
  { 0.000006977,  -000.052519917,   00130.502043649,  -0106717.798479092},
  { 0.000007348,  -000.054635217,   00134.115953542,  -0108372.018430622},
  { 0.000007749,  -000.056900972,   00137.966382976,  -0110143.908673328},
  { 0.000008183,  -000.059331673,   00142.074271369,  -0112042.950017841},
  { 0.000008654,  -000.061943462,   00146.462697347,  -0114079.456292247},
  { 0.000009165,  -000.064754327,   00151.157077550,  -0116264.620819116},
  { 0.000009721,  -000.067784309,   00156.185363220,  -0118610.549458585},
  { 0.000010328,  -000.071055719,   00161.578221666,  -0121130.270754356},
  { 0.000010990,  -000.074593358,   00167.369182094,  -0123837.709187370},
  { 0.000012133,  -000.081127216,   00179.404650315,  -0130905.523845797},
  { 0.000013396,  -000.088243525,   00192.319061560,  -0138374.816834502},
  { 0.000014794,  -000.096001364,   00206.188139413,  -0146274.225955294},
  { 0.000016343,  -000.104466022,   00221.093917306,  -0154633.941675300},
  { 0.000018060,  -000.113709472,   00237.124847304,  -0163485.488607398},
  { 0.000019965,  -000.123810774,   00254.375680819,  -0172861.347748051},
  { 0.000022082,  -000.134856336,   00272.946994362,  -0182794.350272176},
  { 0.000024433,  -000.146939913,   00292.944175723,  -0193316.745455906},
  { 0.000027047,  -000.160162201,   00314.475603055,  -0204458.805668301},
  { 0.000029955,  -000.174629758,   00337.649629860,  -0216246.775523160},
  { 0.000028656,  -000.164485479,   00313.063261672,  -0197300.576745729},
  { 0.000026488,  -000.149639482,   00280.196476858,  -0173627.852182335},
  { 0.000023199,  -000.128919305,   00237.275622128,  -0144367.512090350},
  { 0.000018519,  -000.101120709,   00182.595504385,  -0108762.978393978},
  { 0.000012222,  -000.065376606,   00115.177818014,  -0066547.968700139},
  { 0.000004287,  -000.021950734,   00036.110853596,  -0018692.703771137},
  {-0.000004842,   000.026482157,  -00049.404982833,   0031518.896451924},
  {-0.000014005,   000.073758010,  -00130.564322381,   0077835.430977417},
  {-0.000021524,   000.111520322,  -00193.594949825,   0112756.107073415},
  {-0.000026295,   000.134784530,  -00231.162597229,   0132791.179295963},
  {-0.000038032,   000.188175644,  -00311.305117020,   0172342.401729367},
  {-0.000056089,   000.268053466,  -00428.055764777,   0228561.244577522},
  {-0.000084386,   000.389714065,  -00601.056260553,   0309738.731660332},
  {-0.000129135,   000.576468718,  -00859.025841274,   0427458.120826437},
  {-0.000198788,   000.857940791,  -01235.601726193,   0593970.489433957},
  {-0.000298797,   001.246986192,  -01736.230983505,   0806666.209704303},
  {-0.000406172,   001.640025310,  -02209.161788793,   0992889.277618721},
  {-0.000385320,   001.510709637,  -01976.577402597,   0863125.662719564},
  { 0.000255855,  -000.925745034,   01111.647018142,  -0442674.852399813},
  { 0.002874970,  -010.301591409,   12298.221313884,  -4891390.940983547},
  
};

const float qmax_rpm[KRPM_Length] = {
  2.6*615.2495, 2.6*613.0780, 2.6*610.9066, 2.6*608.7351, 2.6*606.5636, 2.6*604.3922,
  2.6*602.2207, 2.6*600.0492, 2.6*597.8778, 2.6*595.7063, 2.6*593.5348, 2.6*587.0204,
  2.6*580.5060, 2.6*573.9916, 2.6*567.4772, 2.6*560.9628, 2.6*554.4484, 2.6*547.9340,
  2.6*541.4196, 2.6*534.9052, 2.6*528.3908, 2.6*524.7716, 2.6*521.1525, 2.6*517.5334,
  2.6*513.9143, 2.6*510.2952, 2.6*506.6761, 2.6*503.0569, 2.6*499.4378, 2.6*495.8187,
  2.6*492.1996, 2.6*484.2376, 2.6*476.2755, 2.6*468.3134, 2.6*460.3514, 2.6*452.3893,
  2.6*444.4273, 2.6*436.4652, 2.6*428.5032, 2.6*420.5411, 2.6*412.5791, 2.6*405.3409,
  2.6*398.1026, 2.6*390.8644, 2.6*383.6262, 2.6*376.3879, 2.6*369.1497, 2.6*361.9115,
  2.6*354.6732, 2.6*347.4350, 2.6*340.1968, 2.6*329.3394, 2.6*318.4821, 2.6*307.6248,
  2.6*296.7674, 2.6*285.9101, 2.6*275.0527, 2.6*264.1954, 2.6*253.3380, 2.6*242.4807,
  2.6*231.6233
};  
//---------------------- fine mappatura motore Quantum ------------------------

//--------------------------------------------------------------------
// Elabora il coefficente di sbilanciameoto motori R/F in funzione 
// della differenza di temperature. I dati sono ricavati da un simulatore
//--------------------------------------------------------------------
float coeff_sbil(short Tf, short Tr)
{
  
  int dt;
  float value_ret = 1.0;  // % di sbilanciamento tra FanR & FanF < 1 FanF diminusce
  
  dt = Tr-Tf;
  
  if (Tf <= -50 && Tf >-100)       // per temperatura in ingresso compresa tra -5.0 C & -10.0 C
  {
      if (dt > 250)   // differenza di temp. > di 25.0
        value_ret = 0.93;   // i motori F vanno al 93% rispetto quelli R 100%  
      if (dt <= 250 && dt > 225) 
        value_ret = 0.85;
      if (dt <= 225 && dt > 200) 
        value_ret = 0.66;   
      if (dt <= 200) 
        value_ret = 0.59;
  }
  else if (Tf <= -100 && Tf>-150) // per temperatura in ingresso compresa tra -10.0 C & -15.0 C
  {
      if (dt > 300) 
        value_ret = 0.88;    
      if (dt <= 300 && dt>250) 
        value_ret = 0.75;
      if (dt <= 250 && dt>200) 
        value_ret = 0.5;   
      if (dt <= 200) 
        value_ret = 0.25;
  }
  else if (Tf <= -150)            // per temperatura in ingresso minore di -15.0 C
  {
      if (dt > 350) 
        value_ret = 0.75;    
      if (dt <= 350 && dt>325) 
        value_ret = 0.55;
      if (dt <= 325 && dt>300) 
        value_ret = 0.44;   
      if (dt <= 300) 
        value_ret = 0.33;
    }
    
  return value_ret; 
}

//--------------------------------------------------------------------
// Funzione defrost (sbrinatore)
//--------------------------------------------------------------------
void deFrostFunc()
{
   Byte nNtc = 2;

  
   nNtc = read_byte_eeprom(ADDR_EEP(numNTC));
   
   if(read_byte_eeprom(ADDR_EEP(SerialString[7])) == '6')
		return;
   
   
   if (AccessoryPresent_EepList(ACC_EEP_DXD) && sData.RegDXD.defrost_on)
   {
#ifdef VIEW_MOTORSPEED_DBG  //''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''' x_dbg           
       Serial_println1(" <<< Defrost by DXD >>>");  
#endif  //''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''' x_dbg 
       // facciamo uno sbilanciamento, il motore di supply si riduce del 60%                 
       temp_speed_motorsF =(int)(0.4 * ((float)temp_speed_motorsR));
       
       if (temp_speed_motorsF < 51) {
         temp_speed_motorsR += (51 - temp_speed_motorsF);
         temp_speed_motorsF = 51; 
       }   
       
       sData.status_unit |= (1 << POS_BIT_DEFROST_OPERATING); 
       count_sec_defrost = (SOGLIA_CICLO_DEFROST + 1);
       
       return;
   }  
   
   
   
   if ((sData.status_unit & (1 << POS_BIT_DEFROST_OPERATING)) && !(sData.status_test & MSK_TEST_ENABLE)) 
   {
     
#ifdef VIEW_MOTORSPEED_DBG  //''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''' x_dbg  
       Serial_println1(" "); 
       Serial_print1("DeFrost ON: ");           
       Serial_print(count_sec_defrost, DEC);
#endif  //''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''' x_dbg     
    
          
       if(count_sec_defrost > SOGLIA_CICLO_DEFROST) // 20 min. max
         sData.status_unit &= ~(1 << POS_BIT_DEFROST_OPERATING); 
       else {
           // Calcola lo sbilanciamento e, verifica le condizioni per disattivare il defrost 
           switch(nNtc) {
               case 4:
                  if(CkAlarm(ALM_PTEXA_KO) || CkAlarm(ALM_MR1_NO_PWR) || CkAlarm(ALM_MF1_NO_PWR) || CkAlarm(ALM_MR1_NO_PULSE) || CkAlarm(ALM_MF1_NO_PULSE)) 
				  {
                    sData.status_unit &= ~(1 << POS_BIT_DEFROST_OPERATING); 
					//digitalWrite(pIPEHD, LOW);
					ioport_set_pin_level(pIPEHD, IOPORT_PIN_LEVEL_LOW);
                    return;
                  }  
                  
                  if(sData.measure_Temp[I_PROBE_EXHAUST] >= 50) {
                     sData.status_unit &= ~(1 << POS_BIT_DEFROST_OPERATING);
                     persist_temp_defrost_on = 0;
					 //digitalWrite(pIPEHD, LOW);
					 ioport_set_pin_level(pIPEHD, IOPORT_PIN_LEVEL_LOW);
                  }else {
                     // ogni minuto verifico lo sbilanciamento dei motori in funzione della temp. di EXUST 
                     if((count_sec_defrost % 50) == 0)  { // 50 * 1,2 = 60 sec.
                       if(DigitAccessoryOn(ACC_I2C_PREHEATER)) {
                          unbalance_deFrost = 0.85;   
                       }else if (sData.measure_Temp[I_PROBE_EXHAUST] <= -60) 
						{
                          unbalance_deFrost = 0.35;   // Motor F al 35% rispetto al Motor R
						  //digitalWrite(pIPEHD, HIGH);
						  ioport_set_pin_level(pIPEHD, IOPORT_PIN_LEVEL_HIGH);
						}
                       else if (sData.measure_Temp[I_PROBE_EXHAUST] < -40) 
					   {
                          unbalance_deFrost = 0.45;   // Motor F al 45% rispetto al Motor R
						  //digitalWrite(pIPEHD, HIGH);
						  ioport_set_pin_level(pIPEHD, IOPORT_PIN_LEVEL_HIGH);
					   }
                       else if(sData.measure_Temp[I_PROBE_EXHAUST] < -20)
					   {
                          unbalance_deFrost = 0.60;   // Motor F al 60% rispetto al Motor R
						  //digitalWrite(pIPEHD, HIGH);
						  ioport_set_pin_level(pIPEHD, IOPORT_PIN_LEVEL_HIGH);
					   }
                       else if (sData.measure_Temp[I_PROBE_EXHAUST] <  0)
					   {
                          unbalance_deFrost = 0.75;   // Motor F al 75% rispetto al Motor R
						  //digitalWrite(pIPEHD, HIGH);
						  ioport_set_pin_level(pIPEHD, IOPORT_PIN_LEVEL_HIGH);
					   }
                       else if (sData.measure_Temp[I_PROBE_EXHAUST] < 30) 
					   {
                          unbalance_deFrost = 0.90;   // Motor F al 85% rispetto al Motor R
						  //digitalWrite(pIPEHD, LOW);
						  ioport_set_pin_level(pIPEHD, IOPORT_PIN_LEVEL_LOW);
					   }
                     }                        
                  }  

#ifdef VIEW_MOTORSPEED_DBG  //''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''' x_dbg  
                  Serial_print1(" TE:");  
                  Serial_print((float)sData.measure_Temp[I_PROBE_EXHAUST]/10, 1);
#endif  //''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''' x_dbg                                    
               break;
               
               case 3:
                 // TODO: deFrost con 3 sonde               
               default:
                  if(CkAlarm(ALM_PTRET_KO) || CkAlarm(ALM_PTFRESH_KO) || (sData.measure_Temp[I_PROBE_FRESH] >= -4)) {
                    sData.status_unit &= ~(1 << POS_BIT_DEFROST_OPERATING); 
                    persist_temp_defrost_on = 0;
                    return;
                  }  
                  
                  unbalance_deFrost = coeff_sbil(sData.measure_Temp[I_PROBE_FRESH], sData.measure_Temp[I_PROBE_RETURN]);        
 
#ifdef VIEW_MOTORSPEED_DBG  //''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''' x_dbg  
                  Serial_print1(" TF:");  
                  Serial_print((float)sData.measure_Temp[I_PROBE_FRESH]/10, 1);
                  Serial_print1(" TR:");  
                  Serial_print((float)sData.measure_Temp[I_PROBE_RETURN]/10, 1);
#endif  //''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''' x_dbg                    
               break;     
           }
           
           if(unbalance_deFrost < 0.25)
             unbalance_deFrost = 0.25;  
           else if (unbalance_deFrost > 1.0)  
             unbalance_deFrost = 1.0;
                        
           temp_speed_motorsF =(int)(unbalance_deFrost * ((float)temp_speed_motorsR));
           
           if (temp_speed_motorsF < 51) {
             temp_speed_motorsR += (51 - temp_speed_motorsF);
             temp_speed_motorsF = 51; 
           }   
     
           count_sec_defrost++;       
       }  
                
#ifdef VIEW_MOTORSPEED_DBG  //''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''' x_dbg  
       Serial_print1(" Sbil: ");           
       Serial_println(unbalance_deFrost,  2);
#endif  //''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''' x_dbg                                         
    
   }else {   
        if(count_sec_defrost) {
           count_sec_defrost--; // finche' il contatore non va a 0 l'unita' lavora normalmente
           
#ifdef VIEW_MOTORSPEED_DBG  //''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''' x_dbg  
           Serial_print1("DeFrost OFF: ");           
           Serial_print(count_sec_defrost, DEC);
#endif  //''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''' x_dbg       
        }else {
           // Verifica le condizioni per attivare il defrost, in base alle sonde presenti 
           switch(nNtc) {
             case 4:
                if(sData.measure_Temp[I_PROBE_EXHAUST] <= 30) {
                  if(persist_temp_defrost_on++ > 3) {
                     sData.status_unit |= (1 << POS_BIT_DEFROST_OPERATING); 
                     persist_temp_defrost_on = 4;
                  }  
                }else
                  persist_temp_defrost_on = 0;  
             break;
             
             case 3: // TODO: deFrost con 3 sonde                              
             default:
                if(sData.measure_Temp[I_PROBE_FRESH] <= SOGLIA_TF_DEFROST) {
                  if(persist_temp_defrost_on++ > 3) { 
                      sData.status_unit |= (1 << POS_BIT_DEFROST_OPERATING);
                      persist_temp_defrost_on = 4;
                  }    
                }else
                  persist_temp_defrost_on = 0;                
             break;     
           }         
           
           unbalance_deFrost = 1.0;     
        }        
   }  
} 

//--------------------------------------------------------------------
//  CALCOLA DEPOTENZIAMENTO:  
//
//  il depotenziamento varia da 100% a 40%.
//
//--------------------------------------------------------------------
float calcDepot()
{                                 
  float ret_coef = 1.0, depot; 
  
  depot = (float)read_byte_eeprom(ADDR_EEP(depotMotors));
  
  ret_coef = depot / 100;
  
  if(ret_coef < 0.4)
     ret_coef = 0.4; // sotto il 40% non andiamo
   
  return ret_coef;   
}

//------------------------------------------------------------------------------
// Func:
//    ctrl_comand_inputs
//
// Descrizione:
//   Verifichiamo le impostazioni degli input:
//    A ) se un input Ã¨ configurato come FC (INP_0V_UNIT_STOP | INP_12V_UNIT_STOP),
//        e l'ingresso ci dice di fermare l'unitÃ , ferma l'unitÃ  e basta.
//    B ) se un input Ã¨ configurato come Regul. Air Flow e l'altro input configurato 
//        come FC precendenza all'input FC, poi all'altro.
//   
// Values Return:
//      0x84 = Input Config as Allarme Incendio & Extract OFF, Supply MAX.  // Dalla 2.15 
//      0x83 = Input Config as Allarme Incendio & Extract Max, Supply OFF.  // Dalla 2.15   
//      0x82 = Input Config as Allarme Incendio & Extract & Supplay Max.    // Dalla 2.15
//      0x81 = Input Config as Reg_AirFlow/ Allarme incedio & Unit Stop. 
//      0x80 = Input Config as Reg_AirFlow & Unit RUN
//      0x11 = Input Config as (UNIT_ON/OFF | Allarme incendio) & Unit Stop 
//      0x10 = Input Config as UNIT_ON/OFF & Unit RUN   
//      0x00 = Input NO Config 
//------------------------------------------------------------------------------
byte ctrl_comand_inputs ()
{
	byte action_input = 0x00;
	int Analogic_speedMotors = 0;
	byte val_inp[2];
	byte m_reset;
	byte PwrOff;
	unsigned short enab_func; 
	byte Temp;
	
	// Definizione configurazioni inputs in eeprom
	// 0= Disable
	// 1= 12V->Unit RUN , 0V->Unit STOP
	// 2= 0V->Unit RUN , 12V->Unit STOP
	// 3= 0-10V Air flow regulation
	// 4= 12V->Bypass Open ,  0V->Bypass Closed
	// 5= 0V->Bypass Open , 12V->Bypass Closed
	// 7= 0V-> SUMMER ENABLE , 12V -> WINTER ENABLE
	// 8= 0V-> WINTER ENABLE , 12V -> SUMMER ENABLE
	// 9= PIR Strategy
	// ...
	// 11= 12V-> Unit STOP + messaggio: 'ALARM Incendio Ventilazione ferma'
	// 12= 12V-> Ventilazione MAX + messaggio:'ALARM Incendio Ventilazione al massimo'
	// 13= 12V-> Vent. Estrazione al Massimo, Immisione spenta + messaggio: 'ALARM Incendio Solo estrazione aria'
	// 14= 12V-> Vent. Immisione al Massimo, Estrazione spenta + messaggio: 'ALARM Incendio Solo estrazione aria'
	
	val_inp[0] = read_byte_eeprom(ADDR_EEP(Set_Input[0]));
	val_inp[1] = read_byte_eeprom(ADDR_EEP(Set_Input[1]));
	m_reset    = read_byte_eeprom(ADDR_EEP(manual_reset));
	

	if((val_inp[0]==INP_12V_UNIT_STOP) || (val_inp[0]==INP_0V_UNIT_STOP) || (val_inp[1]==INP_12V_UNIT_STOP) || (val_inp[1]==INP_0V_UNIT_STOP) || (val_inp[0]==INP_PIR) || (val_inp[1]==INP_PIR))
		action_input = 0x10;
	else if  ((val_inp[0]==INP_REG_AIR_FLOW) || (val_inp[1]==INP_REG_AIR_FLOW))
			action_input = 0x80;
			else
				action_input = 0x00;
	
	// segnaliamo che la velocita' dei motori e' generato dallo stato degli input
	if(action_input & 0x80)
	sData.status_unit  |= (word)(1<< POS_BIT_CMD_FAN_INPUT);
	else
	sData.status_unit  &= ~(word)(1<< POS_BIT_CMD_FAN_INPUT);
	
	
	for(int i=0; i < 2; i++) {
		if (val_inp[i] == INP_0V_UNIT_STOP) {
			if (((PowerMotors == POWER_SAVE) && (sData.measure_ADC_input[i] < 750))  || ((PowerMotors == POWER_ON)   && (sData.measure_ADC_input[i] < 200)))
				action_input = 0x11;
				break;
			}else if (val_inp[i] == INP_12V_UNIT_STOP) {
				if (((PowerMotors == POWER_SAVE) && (sData.measure_ADC_input[i] >= 200)) || ((PowerMotors == POWER_ON)   && (sData.measure_ADC_input[i] >= 750)))
					action_input = 0x11;
					break;
			}else if (val_inp[i] == INP_REG_AIR_FLOW) {

			// verifichiamo la soglia di funzionamento motori.
			Analogic_speedMotors = ((sData.measure_ADC_input[i] + 3) >> 2); // V_input_max = 10.0V
			
			if(Analogic_speedMotors >= 52) {
				if(Analogic_speedMotors > 255)
				Analogic_speedMotors = 255;
				
				temp_speed_motorsR = Analogic_speedMotors;
				temp_speed_motorsF = Analogic_speedMotors;
				action_input = 0x80;  // ON
			}else
			action_input = 0x81; // OFF
			
			break;
			}else if((val_inp[i] >= INP_12V_ALM_FIRE_UNIT_STOP) && (val_inp[i] <= INP_12V_ALM_FIRE_ONLY_SUPPLY)) {  // Entro nel caso: è scattato l'allarme incendio (indipendentemente dalla modalità scelta)
			//----------------------------------------------------------
			//  La gestione dell'allarme incendio ha priorità su tutto.
			//----------------------------------------------------------
			if(val_inp[i] == INP_12V_ALM_FIRE_UNIT_STOP){
				if (sData.measure_ADC_input[i] >= 350)
				{
					byte cntUpdate_SettingPar;

					action_input = 0x81;
					// attiva l'allarme
					RemoveAlarm(ALM_SMOKE_VENT_MAX);
					RemoveAlarm(ALM_SMOKE_ONLY_SUP);
					RemoveAlarm(ALM_SMOKE_ONLY_EXT);
					
					InsertAlarm(ALM_SMOKE_VENT_OFF);
					if (m_reset == 0)
					{
						cntUpdate_SettingPar = read_byte_eeprom( ADDR_EEP( cntUpdate_SettingPar ) );
						cntUpdate_SettingPar++;
						write_byte_eeprom(ADDR_EEP(manual_reset),1); //Alzo il riarmo manuale
						write_byte_eeprom( ADDR_EEP( cntUpdate_SettingPar ), cntUpdate_SettingPar );
					}
					break;
				}else if( (sData.measure_ADC_input[i] < 350)) //Per rimuovere l'allarme deve esserci un riarmo manuale, aspetto che il KTS mi dia l'ok (manual_reset =  0)
				{
					if( m_reset == 1)
					{
						action_input = 0x81;   //tengo i motori fermi fintanto che non riarmo
					}else
					RemoveAlarm(ALM_SMOKE_VENT_OFF);
				}
				
				}else if (val_inp[i] == INP_12V_ALM_FIRE_VENT_MAX){
				if(sData.measure_ADC_input[i] >= 400) {
					temp_speed_motorsR = 255;
					temp_speed_motorsF = 255;
					action_input = 0x82;  // Vent Extract: MAX, Vent. Supply: MAX
					
					RemoveAlarm(ALM_SMOKE_VENT_OFF);
					RemoveAlarm(ALM_SMOKE_ONLY_SUP);
					RemoveAlarm(ALM_SMOKE_ONLY_EXT);
					// attiva l'allarme
					InsertAlarm(ALM_SMOKE_VENT_MAX);
					break;
				}else
				RemoveAlarm(ALM_SMOKE_VENT_MAX);
				
				}else if (val_inp[i] == INP_12V_ALM_FIRE_ONLY_EXTRACT){
				if(sData.measure_ADC_input[i] >= 400) {
					temp_speed_motorsR = 255;
					temp_speed_motorsF = 0;
					action_input = 0x83;  // Vent Extract: MAX, Vent. Supply: OFF
					
					RemoveAlarm(ALM_SMOKE_VENT_OFF);
					RemoveAlarm(ALM_SMOKE_VENT_MAX);
					RemoveAlarm(ALM_SMOKE_ONLY_SUP);
					// attiva l'allarme
					InsertAlarm(ALM_SMOKE_ONLY_EXT);
					break;
				}else
				RemoveAlarm(ALM_SMOKE_ONLY_EXT);
				
				}else if (val_inp[i] == INP_12V_ALM_FIRE_ONLY_SUPPLY){
				if(sData.measure_ADC_input[i] >= 400) {
					temp_speed_motorsR = 0;
					temp_speed_motorsF = 255;
					action_input = 0x84;  // Vent Extract: OFF, Vent. Supply: MAX
					
					RemoveAlarm(ALM_SMOKE_VENT_OFF);
					RemoveAlarm(ALM_SMOKE_VENT_MAX);
					RemoveAlarm(ALM_SMOKE_ONLY_EXT);
					// attiva l'allarme
					InsertAlarm(ALM_SMOKE_ONLY_SUP);
					break;
				}else
				RemoveAlarm(ALM_SMOKE_ONLY_SUP);
			}
			
		}
	}
	
	wdt_restart(WATCH_DOG);
	
	enab_func = read_word_eeprom(ADDR_EEP(Enab_Fuction)); 
	
	if (val_inp[1] == INP_SUMMER_WINTER)
	{
		if (sData.measure_ADC_input[1] > 150) // WINTER
		{
			if((DigitAccessoryOperating(ACC_I2C_DXD) != 0) || (DigitAccessoryOperating(ACC_I2C_HEATER) != 0) || (DigitAccessoryOperating(ACC_I2C_SSR) != 0)) // ENABLE WINTER
			{
				if (!(enab_func &  (1 << ENAB_WINTER))) // WINTER NOT ENABLED
				{
					enab_func |= (1 << ENAB_WINTER);
					enab_func &= ~(1 << ENAB_SUMMER);
					write_word_eeprom(ADDR_EEP(Enab_Fuction), enab_func);
					Temp = read_byte_eeprom(ADDR_EEP(cntUpdate_info));
					Temp++;
					write_byte_eeprom(ADDR_EEP(cntUpdate_info), Temp);
				}
			}
		}
		else // SUMMER
		{
			if((DigitAccessoryOperating(ACC_I2C_DXD) != 0) || (DigitAccessoryOperating(ACC_I2C_COOLER) != 0)) // ENABLE SUMMER
			{
				if (!(enab_func &  (1 << ENAB_SUMMER))) // SUMMER NOT ENABLED
				{
					enab_func |= (1 << ENAB_SUMMER);
					enab_func &= ~(1 << ENAB_WINTER);
					write_word_eeprom(ADDR_EEP(Enab_Fuction), enab_func);
					Temp = read_byte_eeprom(ADDR_EEP(cntUpdate_info));
					Temp++;
					write_byte_eeprom(ADDR_EEP(cntUpdate_info), Temp);
				}
			}
		}
		
	}
	else
		if (val_inp[1] == INP_WINTER_SUMMER)
		{
			if (sData.measure_ADC_input[1] > 150) // SUMMER
			{
				if((DigitAccessoryOperating(ACC_I2C_DXD) != 0) || (DigitAccessoryOperating(ACC_I2C_COOLER) != 0)) // ENABLE SUMMER
				{
					if (!(enab_func &  (1 << ENAB_SUMMER))) // SUMMER NOT ENABLED
					{
						enab_func |= (1 << ENAB_SUMMER);
						enab_func &= ~(1 << ENAB_WINTER);
						write_word_eeprom(ADDR_EEP(Enab_Fuction), enab_func);
						Temp = read_byte_eeprom(ADDR_EEP(cntUpdate_info));
						Temp++;
						write_byte_eeprom(ADDR_EEP(cntUpdate_info), Temp);
					}
				}
			}
			else // WINTER
			{
				if((DigitAccessoryOperating(ACC_I2C_DXD) != 0) || (DigitAccessoryOperating(ACC_I2C_HEATER) != 0) || (DigitAccessoryOperating(ACC_I2C_SSR) != 0)) // ENABLE WINTER
				{
					if (!(enab_func &  (1 << ENAB_WINTER))) // WINTER NOT ENABLED
					{
						enab_func |= (1 << ENAB_WINTER);
						enab_func &= ~(1 << ENAB_SUMMER);
						write_word_eeprom(ADDR_EEP(Enab_Fuction), enab_func);
						Temp = read_byte_eeprom(ADDR_EEP(cntUpdate_info));
						Temp++;
						write_byte_eeprom(ADDR_EEP(cntUpdate_info), Temp);
					}
				}
			}
		}
	
	for(int i=0; i < 2; i++)
	{
		if (val_inp[i] == INP_PIR)
		{
			if ( Pir_Startup )
			{
				if ((pir_counter >= 76000) && !pir_counter_flag)
				{
					pir_counter_flag = true;
					Pir_Startup = false;
					PIR_Delay_Counter = 0;
				}
			}
			else
				if ((pir_counter >= 5000) && !pir_counter_flag)
				{
					pir_counter_flag = true;
					PIR_Delay_Counter = 0;
				}
				
			PwrOff = read_byte_eeprom(ADDR_EEP(Set_Power_ON)); // 1 = power_on, 0 = standby	
			
			if (PwrOff == 0) // Power Off
			{
				action_input = 0x11;
				sData.status_unit &= ~(word)(1 << POS_BIT_PIR_CHECK);
				pir_counter = 0;
				pir_counter_flag = false;
				Power_On_Already_Activated = false;
				PIR_Delay_Counter = 0;
			}
			else // Power On
			{
				// WEEKLY attivo: salta PIR se in RUN, altrimenti esegui PIR
				if ((sData.status_weekly & 0x02) != 0x2 ) // weekly not running , pir is activated
				{
					// L'unità è in STANDBY ? esegui PIR check
					if (sData.measure_ADC_input[i] > 150) // input i > 150
					{
						if (pir_counter_flag && !Power_On_Already_Activated)
						{
							Power_On_Already_Activated = true;
							sData.status_unit |= (word)(1 << POS_BIT_PIR_CHECK);
							pir_counter = 0;
							action_input = 0x10;
							PIR_Delay_Counter = 0;
						}
					}
					else // input i < 150
					{
						sData.status_unit  &= ~(word)(1 << POS_BIT_PIR_CHECK);
						Power_On_Already_Activated = false;
						if (( PIR_Delay_Counter >= (read_byte_eeprom(ADDR_EEP(Pir_Update_Delay)) * 60000)) && pir_counter_flag ) // 60000
						{
							action_input = 0x11;
							PIR_Delay_Counter = 0;
						}
						
						if (((sData.status_unit & MSK_UNIT_OPERATING) == 0) && pir_counter_flag)
							action_input = 0x11;
					}
				}
				else // weekly is runnig , pir is desactivated
				{
					action_input = 0x10;
					pir_counter = 0;
					pir_counter_flag = false;
					Power_On_Already_Activated = false;
					sData.status_unit  &= ~(word)(1 << POS_BIT_PIR_CHECK);
					PIR_Delay_Counter = 0;
				}
			}	
		}
	}
	
	if((action_input && 0x01) == 0x01)
	{
		sData.status_unit  &= ~(word)(1 << POS_BIT_UNIT_RUN); // OFF
	}
	else
	{
		sData.status_unit |= (word)(1 << POS_BIT_UNIT_RUN);  // ON
	}
	
	// Funzione fire disabilitata, togliamo gli allarmi
	if(action_input < 0x81)
	{
		RemoveAlarm(ALM_SMOKE_VENT_OFF);
		RemoveAlarm(ALM_SMOKE_VENT_MAX);
		RemoveAlarm(ALM_SMOKE_ONLY_EXT);
		RemoveAlarm(ALM_SMOKE_ONLY_SUP);
	}
	
	return action_input;
}


//-----------------------------------------------------------------------------
// Porta l'unitÃ  in standby
//-----------------------------------------------------------------------------
void switchesToStandby ()
{
    byte buff[8];
    word addr;

  
    // Prima di spegnere l'impianto controlliamo che vi sono elementi di riscaldamento attivi:    
    // Se si, attiviamo la  modalita' di post-ventilazione (durata max 4 minuti), cosÃ¬
    // da evitare che l'inerzia termica degli accesori danneggi l'impianto.   
    if (((DigitAccessoryOn(ACC_I2C_HEATER) && ((sData.AccI2C[ACC_I2C_HEATER].sts & STS_ACC_ELECT)))|| (DigitAccessoryOn(ACC_I2C_PREHEATER) && ((sData.AccI2C[ACC_I2C_PREHEATER].sts & STS_ACC_ELECT)))|| DigitAccessoryOn(ACC_I2C_COOLER)) 
        && (count_post_vent == 0) 
        && (sData.status_unit & MSK_UNIT_OPERATING)) 
    {
      count_post_vent = 150; // 150 *1.2 = 180 sec.     
    }   
     
    if(count_post_vent) {
      TAG_DigitAccessoryOff(ACC_I2C_PREHEATER); 
      TAG_DigitAccessoryOff(ACC_I2C_HEATER); 
      TAG_DigitAccessoryOff(ACC_I2C_COOLER); 
      
      count_post_vent--;
      sData.status_unit &= ~MSK_UNIT_OPERATING; // STANDBY 
      
      if (count_post_vent )     
        sData.status_unit |= (1 << POS_BIT_POST_VENT_OPERATING); 

      
      if (count_post_vent <= 125) // 125 * 1.2 = 150 sec. 
      { 
          // per 30 secondi mantieni la velocitÃ  come era, poi decrementa
          sData.speed_motors_R--;
          sData.speed_motors_F--;
          if(sData.speed_motors_R < 85)
            sData.speed_motors_R = 85; // minimo garantito: > 33%  
          if(sData.speed_motors_F < 85)
            sData.speed_motors_F = 85; // minimo garantito: > 33%  
            
          temp_speed_motorsR = sData.speed_motors_R;
          temp_speed_motorsF = sData.speed_motors_F;  
      }
      
#ifdef VIEW_MOTORSPEED_DBG  //''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''' x_dbg  
      Serial_print1("PostVent: ");
      Serial_print(count_post_vent, DEC);
      Serial_print1(" R: ");
      Serial_print(sData.speed_motors_R, DEC);
      Serial_print1(" F: ");
      Serial_println(sData.speed_motors_F, DEC);
#endif  //''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''' x_dbg       
      
    }else { 
    
        count_switch_on    = 0;   
        temp_speed_motorsR = 0;
        temp_speed_motorsF = 0;
        sData.speed_motors_R = temp_speed_motorsR;
        sData.speed_motors_F = temp_speed_motorsF;
        sData.status_unit  &= ~MSK_UNIT_OPERATING; // STANDBY   
        
        count_switch_off++;
                     
        if (count_switch_off == 7) 
        {  
          
           PowerMode(POWER_SAVE);  
         
           // salviamo in eeprom le ore di funz.
           buff[0] = (byte)countHoursFunz;             // byte Low
           buff[1] = (byte)(countHoursFunz >>  8);
           buff[2] = (byte)(countHoursFunz >> 16);
           buff[3] = (byte)(countHoursFunz >> 24);     // byte high
           
           addr = ADDR_EEP(hour_runnig); 
            
           update_data_eeprom(addr, (addr+3), &buff[0]);  // 4 byte            
        }  
        
        if(count_switch_off > 12)
           count_switch_off = 12;
    }   
}  

//-----------------------------------------------------------------------------
// Calcola Q
//-----------------------------------------------------------------------------
#ifdef COMESSA_2388
float CalcQW_by_PW( byte speed_r, unsigned short pw)
{
  float QW = 1;
  float MaxQW = 1200.0;

  QW =  (double)gKVolts[ 100 - speed_r ][0] * (double)pw * (double)pw * pw;  
  QW += (double)gKVolts[ 100 - speed_r ][1] * (double)pw * pw;
  QW += (double)gKVolts[ 100 - speed_r ][2] * pw;
  QW += gKVolts[ 100 - speed_r ][3];
    
  MaxQW = (float)GetMaxAirFlow() * 1.07;
  
  if( QW  > MaxQW)
    QW = MaxQW;
  else if ( QW  < 10) 
    QW = 10;
  
  return QW;  
  
}  
#endif


//-----------------------------------------------
// Funzione Standard
//-----------------------------------------------
float CalcQW( byte speed_r, unsigned short rpm)
{
  float QW = 1;
  float MaxQW = 575.0;

  QW = (gKRPM[ 100 - speed_r ][ 0 ] * (double)rpm * (double)rpm * rpm) +
       (gKRPM[ 100 - speed_r ][ 1 ] * (double)rpm * rpm) +
       (gKRPM[ 100 - speed_r ][ 2 ] * rpm) + gKRPM[ 100 - speed_r ][ 3 ];
    
  MaxQW = (float)GetMaxAirFlow() * 1.07;
  
  if( QW  > MaxQW)
    QW = MaxQW;
  else if ( QW  < 10) 
    QW = 10;
  
  return QW;
}

//-----------------------------------------------------------------------------
// Calcola P
//-----------------------------------------------------------------------------
float CalcPW(byte speed_r, float qw)
{
  double pw = 0;
    
  pw =  (double)gKVolts[ 100 - speed_r ][0] * (double)qw * (double)qw * qw;  
  pw += (double)gKVolts[ 100 - speed_r ][1] * (double)qw * qw;
  pw += (double)gKVolts[ 100 - speed_r ][2] * qw;
  pw +=         gKVolts[ 100 - speed_r ][3];
  
  return (float)pw; 
}

//-----------------------------------------------------------------------------
// Ritorna MaxAirFlow
//-----------------------------------------------------------------------------
  
typedef struct _PACK SCLUnitModel
{
 char* Model;
 unsigned short MaxAirFlow;
} CLUnitModel;


CLUnitModel gUnitModels[] =
{
 { (char*)"0025", 300 },
 { (char*)"0035", 400 },
 { (char*)"0045", 450 },
 { (char*)"0055", 550 },
 { NULL, 0 }
};

// Torna la massima portata della macchina
unsigned short GetMaxAirFlow()
{
   short   maxAirFlow;
 
   CLUnitModel* unitModel = gUnitModels;

   while (unitModel->Model != NULL)
   {
     if ((read_byte_eeprom( ADDR_EEP(SerialString) + 4) == unitModel->Model[0]) && 
         (read_byte_eeprom( ADDR_EEP(SerialString) + 5) == unitModel->Model[1]) && 
         (read_byte_eeprom( ADDR_EEP(SerialString) + 6) == unitModel->Model[2]) && 
         (read_byte_eeprom( ADDR_EEP(SerialString) + 7) == unitModel->Model[3]))
    return unitModel->MaxAirFlow;
    unitModel++;
   }
   
   maxAirFlow = ((short) (read_byte_eeprom( ADDR_EEP(SerialString) + 4) - '0')) * 100 + 
		((short) (read_byte_eeprom( ADDR_EEP(SerialString) + 5) - '0')) * 10 +
		((short) (read_byte_eeprom( ADDR_EEP(SerialString) + 6) - '0'));

   if (maxAirFlow < 0)
	maxAirFlow = 0;

   return maxAirFlow * 100;

}

int isDamperLessUnit()
{
	int  isDamperLess = 0;

	if ((read_byte_eeprom( ADDR_EEP(SerialString) + 9) == '9') && (read_byte_eeprom( ADDR_EEP(SerialString) + 10) == '0'))
		isDamperLess = 1;
	else
		isDamperLess = 0;
	
	return isDamperLess;
}

//------------------------------------------------------------------------------
// Gestione ventole a pressione costante:
//    1:  Con accessorio esterno
//    2:  Con calcoli tramite SW
//------------------------------------------------------------------------------
char cnt_delay =0;
char cnt_delay1 =0;

void Func_Pressure_Const(unsigned int setPoint)
{
      short			rpm_r, cnt_loop;
      byte			speed_r	= 20.0;
      const float		errMax	= 5;
      float			qw =10, korr,MaxAirFlow,qSetPoint;
      short	                pw, Kmax_Depot;
      float		        qwNew;
      float			deltaP, deltaP_prev;
      float	                pwCalc;
      signed char		deltaPSign;
      unsigned short            Kmin_reg,Kmax_calcDep;
      int                       speedNew = 51,i; 
   
        
      rpm_r = (sData.rpm_motors_R[0] + rpm_r_preview[0]) >> 1;      
      rpm_r_preview[0] = rpm_r;
      
      // in caso di non rotazione esci dalla funzione 
      if(rpm_r < 120)
         return;
                  
           
   if(DigitAccessoryOperating(ACC_I2C_PCAP)) 
      {

         // avendo un dispositivo che legge direttamente la pressione 
		 // having a device that directly reads the pressure
         // la funzione è molto più semplice
		 // the function is much simpler
         pw =  sData.AccI2C[ACC_I2C_PCAP].measure1;
         
//--------------- correzione limiti di lettura p
          if(pw > 800)
              pw = 800;
         if(pw < 0)
             pw = 10;
             
//--------------- visualizzo p sul kts
         sData.measure_pressure = pw;         
//--------------- calcolo qw

        MaxAirFlow = (float)GetMaxAirFlow();
        qSetPoint  = MaxAirFlow * (float) ((float) setPoint / 1000.0);    
        speed_r    = (byte)((float)sData.speed_motors_R / 2.55); // step di 0.1V            
   
        Kmax_calcDep = KMax * calcDepot();             
        Kmin_reg = KMin;
        
        
        for( i=1; i < KRPM_Length;  i++) {
           if(qSetPoint > qmax_rpm[i])
              break;         
        }  
        	
        Kmin_reg = (100 - i + 1); // Kmin_reg : va da 99 a 40


        korr = 0.000243055*qw*qw*0.8;

        Kmin_reg = (100 - i + 1); // Kmin_reg : va da 99 a 40

         if(speed_r >= Kmin_reg) 
           qw = CalcQW_by_PW(speed_r, pw-korr);
         else {
           qw = CalcQW_by_PW(Kmin_reg, pw) * 0.95;
           }
//--------------- visualizzo qw sul kts
         sData.measure_airflow  = qw;
  
         
//inizia la regolazione          
//---------------- fai la regolazione ogni 9 * 1.2 = 10.8 secondi
         if(++cnt_delay <= 60) 
           return; 
         
//----------------------------------- Nicola 05/05/2016         
// se non c'è stato un aggiornamento dei dati di pressione inutile entrare nel regolatore.
//         if(sData.AccI2C[ACC_I2C_PCAP].measure1 == 0){
//           cnt_delay = 9;
//           return;         
//         }  
//         
//         sData.AccI2C[ACC_I2C_PCAP].measure1 = 0;
//         cnt_delay = 0; 
//------------------------------------------------------------------------ 
// Commentato perchè di poca utilità e crea azzeramenti continui sul KTS
//------------------------------------------------------------------------

         // Impostiamo di default i valori correnti di speed motors
	 speedNew = sData.speed_motors_R; // passo il valore precedente
         
         if(pw < (setPoint - 2)) {      
           int soglie[4];
           
           soglie[0] = (setPoint >> 1);                // -50.0%
           soglie[1] = setPoint - (setPoint >> 2);     // -25.0%
           soglie[2] = setPoint - (setPoint >> 3);     // -12.5%
           soglie[3] = setPoint - (setPoint * 5)/ 100; //  -5.0% 
           
           if(pw < soglie[0]) {       
             speedNew += 1;  // + 10%
             cnt_delay = -30;  // la correzione è grossa occorre apettare 18 sec., visto l'inerzia dei motori
           }else if (pw < soglie[1]) {  
             speedNew += 1;  // + 5%
             cnt_delay = -20;  // la correzione abbastanza grossa occorre apettare: 13.2 sec.
           }else if (pw < soglie[2]) {   
             speedNew += 1;   // + 2%
             cnt_delay = -10;
           }else if (pw <= soglie[3])  
             speedNew++;
             
           if(speedNew > 255)
             speedNew = 255;  
             
         }else if(pw > (setPoint + 2)){
           int soglie[4];
           
           soglie[0] = setPoint + (setPoint >> 1);     // +50.0%
           soglie[1] = setPoint + (setPoint >> 2);     // +25.0%
           soglie[2] = setPoint + (setPoint >> 3);     // +12.5%
           soglie[3] = setPoint + (setPoint * 5)/ 100; //  +5.0% 
           
           if(pw > soglie[0]) {       
             speedNew -= 1;  // + 10%
             cnt_delay = -30;  // la correzione è grossa occorre apettare 18 sec., visto l'inerzia dei motori
           }else if (pw > soglie[1]) {  
             speedNew -= 1;  // + 5%
             cnt_delay = -20;  // la correzione abbastanza grossa occorre apettare: 13.2 sec.
           }else if (pw > soglie[2]) {  
             speedNew -= 1;
             cnt_delay = -10;
           }else if (pw <= soglie[3])  
             speedNew--;
             
           if(speedNew < 52)
             speedNew = 52;            
         }  
        
         temp_speed_motorsR = (speedNew &0x0FF);
         temp_speed_motorsF = temp_speed_motorsR;
      }else {  
        
        speed_r = (float)((float)sData.speed_motors_R + 1.28)/ 2.55;
                       
	  // Calcola punto di lavoro (q, p)
        if(speed_r >= KMin) { 	// 40%
    	    qw  = CalcQW( speed_r, rpm_r );                     
    	    pw	= CalcPW( speed_r, qw );
        }else {
            temp_speed_motorsR = (KMin * 2.55); 
            temp_speed_motorsF =  temp_speed_motorsR;
            return;
        }  
  	
        if(pw < 20) 
          pw = 20;
        
          
	// Impostiamo di default i valori correnti di speed motors
	temp_speed_motorsR = sData.speed_motors_R; // passo il valore precedente
	temp_speed_motorsF = temp_speed_motorsR;

        deltaP		= (float) setPoint - (float) pw;
	deltaPSign	= (deltaP < 0 ? -1 : (deltaP > 0 ? 1 : 0));  

        deltaP_prev     = deltaP;
 
        //entra nella funzione ogni 9 secondi
        cnt_delay++;
        
    
        // trova le misure mediate da riportare al KTS    
        acc_measure_pressure += (unsigned short)pw;
        sample_measure_pressure++;
    
        if(sample_measure_pressure >= 6) {
          sData.measure_pressure  = (unsigned short)(acc_measure_pressure / sample_measure_pressure);
          acc_measure_pressure    >>= 1;
          sample_measure_pressure >>= 1;
        }   
    
        acc_measure_airflow += (unsigned short)qw;
        sample_measure_airflow++;
    
        if(sample_measure_airflow >= 6) {         
          sData.measure_airflow  = (unsigned short)(acc_measure_airflow / sample_measure_airflow);
          sample_measure_airflow >>= 1;
          acc_measure_airflow    >>= 1;
        }          
          
        
        if(cnt_delay < 6) 
          return; 
        
        cnt_delay = 0; 
 
        Kmax_Depot = KMax * calcDepot();       

	if (((deltaP < 0) && (speed_r > KMin)) || ((deltaP > 0) && (speed_r < Kmax_Depot)))
	{
	    unsigned short	speed0, speed1;
	    unsigned short	speedNew;
	    Bool	        speedFound;
            int             reg_fine =  (errMax * 5) + 1;  
		
                           
            if((deltaP_prev >= -reg_fine) && (deltaP_prev <= reg_fine))
            {  
                 // regolazione fine, tra -20pa e +20pa
                 speedNew = sData.speed_motors_R;
                 
                 if (deltaP_prev  >= (errMax+1)) {
                    if(deltaP_prev  > (errMax + 2)) {
                      speedNew += 3;
                      cnt_delay = -1;
                    }else  
                      speedNew++;
                 }else if (deltaP_prev  <= -(errMax+1)){ 
                    if(deltaP_prev  < -(errMax + 2)){
                      speedNew -= 3; 
                      cnt_delay = -1;
                    }else
                      speedNew--;  
                 }   
                    
                 if(speedNew > (Kmax_Depot * 2.55))
                     speedNew = (byte)(Kmax_Depot * 2.55);
                 else if (speedNew < (KMin *2.55))
                     speedNew = (byte)(KMin * 2.55);    
                        
	        temp_speed_motorsR = speedNew; // da 0 a 255
			temp_speed_motorsF = temp_speed_motorsR;                  
            }else {                      
                 if (deltaP > errMax)
		 {
		    speed0   = speed_r;
		    speed1   = Kmax_Depot;
		 }
		 else if (deltaP < -errMax)
		 {
		    speed0   = KMin;
		    speed1   = speed_r;
		 }
                
		 // Calcola Curva di impianto
		 qwNew	= sqrt( (pow( qw, (float) 2 ) / (pw + 1)) * setPoint );

		 // Stima nuovo Livello di regolazione (Diminuisco/Aumento V)
		 speedFound	= false;
                 cnt_loop    = 700;
              
		 while (speed0 <= speed1)
                 {		
	            // Calcola la nuova speed
		    speedNew = speed0 + ((speed1 - speed0) >> 1);

		    // Calcola il nuovo punto di lavoro
		    pwCalc	= CalcPW( speedNew, qwNew );
		    deltaP	= setPoint - pwCalc;
				
		    if (abs( deltaP ) < errMax)
		    {
		        speedFound	= true;
			break;
		    }

		    if (deltaP > 0) 
		      speed0 = speedNew + 1;
		    else{       
                      if (speed1 > KMin)  
		        speed1 = speedNew - 1;
                    }
                      
                    if(cnt_loop) 
                      cnt_loop--;
                    else  
                      break;
		 }

  		 if (speedFound)
  		 {        
                   cnt_delay = -5;
                      
                   if(speedNew > Kmax_Depot)
                     speedNew = Kmax_Depot;
                   else if (speedNew < KMin)
                     speedNew = KMin;    
                          
  	       temp_speed_motorsR = (byte) (speedNew * 2.55); // da 0 a 255
  		   temp_speed_motorsF = temp_speed_motorsR;
  		 }else {
                      
                   // sali
                   if(deltaP_prev  > 40) {
                     speedNew = speed_r + 4;  
                     cnt_delay = -2;                     
                   }else if(deltaP_prev  > 25)
                     speedNew = speed_r + 1;                        
                
                   if(speedNew > Kmax_Depot)
                      speedNew = Kmax_Depot;                       
                  
      		   temp_speed_motorsR = (byte) (speedNew * 2.55); // da 0 a 255
      		   temp_speed_motorsF = temp_speed_motorsR;                  
              }
           }
		}
    }
}

void Func_Pressure_Const_R (unsigned int setPoint )
{
	short			rpm_r, cnt_loop;
	byte			speed_r	= 20.0;
	const float		errMax	= 5;
	float			qw =10, korr,MaxAirFlow,qSetPoint;
	short	                pw, Kmax_Depot;
	float		        qwNew;
	float			deltaP, deltaP_prev;
	float	                pwCalc;
	signed char		deltaPSign;
	unsigned short            Kmin_reg,Kmax_calcDep;
	int                       speedNew = 51,i;
	
	
	rpm_r = (sData.rpm_motors_R[0] + rpm_r_preview[0]) >> 1;
	rpm_r_preview[0] = rpm_r;
	
	// in caso di non rotazione esci dalla funzione
	if(rpm_r < 120)
	return;
	
	
	if(DigitAccessoryOperating(ACC_I2C_FLW2))
	{

		// avendo un dispositivo che legge direttamente la pressione
		// having a device that directly reads the pressure
		// la funzione è molto più semplice
		// the function is much simpler
		pw =  sData.AccI2C[ACC_I2C_FLW2].measure4;
		
		//--------------- correzione limiti di lettura p
		if(pw > 800)
		pw = 800;
		if(pw < 0)
		pw = 10;
		
		//--------------- visualizzo p sul kts
		//sData.measure_pressure = pw;
		Pressure_CAPR = pw;
		//--------------- calcolo qw

		MaxAirFlow = (float)GetMaxAirFlow();
		qSetPoint  = MaxAirFlow * (float) ((float) setPoint / 1000.0);
		speed_r    = (byte)((float)sData.speed_motors_R / 2.55); // step di 0.1V
		
		Kmax_calcDep = KMax * calcDepot();
		Kmin_reg = KMin;
		
		
		for( i=1; i < KRPM_Length;  i++) {
			if(qSetPoint > qmax_rpm[i])
			break;
		}
		
		Kmin_reg = (100 - i + 1); // Kmin_reg : va da 99 a 40


		korr = 0.000243055*qw*qw*0.8;

		Kmin_reg = (100 - i + 1); // Kmin_reg : va da 99 a 40

		if(speed_r >= Kmin_reg)
		qw = CalcQW_by_PW(speed_r, pw-korr);
		else {
			qw = CalcQW_by_PW(Kmin_reg, pw) * 0.95;
		}
		//--------------- visualizzo qw sul kts
		//sData.measure_airflow  = qw;
		Airflow_CAPR = qw;
		
		//inizia la regolazione
		//---------------- fai la regolazione ogni 9 * 1.2 = 10.8 secondi
		if(++cnt_delay1 <= 60)
		return;
		
		//----------------------------------- Nicola 05/05/2016
		// se non c'è stato un aggiornamento dei dati di pressione inutile entrare nel regolatore.
		//         if(sData.AccI2C[ACC_I2C_PCAP].measure1 == 0){
		//           cnt_delay1 = 9;
		//           return;
		//         }
		//
		//         sData.AccI2C[ACC_I2C_PCAP].measure1 = 0;
		//         cnt_delay = 0;
		//------------------------------------------------------------------------
		// Commentato perchè di poca utilità e crea azzeramenti continui sul KTS
		//------------------------------------------------------------------------

		// Impostiamo di default i valori correnti di speed motors
		speedNew = sData.speed_motors_R; // passo il valore precedente
		
		if(pw < (setPoint - 2)) {
			int soglie[4];
			
			soglie[0] = (setPoint >> 1);                // -50.0%
			soglie[1] = setPoint - (setPoint >> 2);     // -25.0%
			soglie[2] = setPoint - (setPoint >> 3);     // -12.5%
			soglie[3] = setPoint - (setPoint * 5)/ 100; //  -5.0%
			
			if(pw < soglie[0]) {
				speedNew += 1;  // + 10%
				cnt_delay1 = -30;  // la correzione è grossa occorre apettare 18 sec., visto l'inerzia dei motori
				}else if (pw < soglie[1]) {
				speedNew += 1;  // + 5%
				cnt_delay1 = -20;  // la correzione abbastanza grossa occorre apettare: 13.2 sec.
				}else if (pw < soglie[2]) {
				speedNew += 1;   // + 2%
				cnt_delay1 = -10;
			}else if (pw <= soglie[3])
			speedNew++;
			
			if(speedNew > 255)
			speedNew = 255;
			
			}else if(pw > (setPoint + 2)){
			int soglie[4];
			
			soglie[0] = setPoint + (setPoint >> 1);     // +50.0%
			soglie[1] = setPoint + (setPoint >> 2);     // +25.0%
			soglie[2] = setPoint + (setPoint >> 3);     // +12.5%
			soglie[3] = setPoint + (setPoint * 5)/ 100; //  +5.0%
			
			if(pw > soglie[0]) {
				speedNew -= 1;  // + 10%
				cnt_delay1 = -30;  // la correzione è grossa occorre apettare 18 sec., visto l'inerzia dei motori
				}else if (pw > soglie[1]) {
				speedNew -= 1;  // + 5%
				cnt_delay1 = -20;  // la correzione abbastanza grossa occorre apettare: 13.2 sec.
				}else if (pw > soglie[2]) {
				speedNew -= 1;
				cnt_delay1 = -10;
			}else if (pw <= soglie[3])
			speedNew--;
			
			if(speedNew < 52)
			speedNew = 52;
		}
		
		temp_speed_motorsR = (speedNew &0x0FF);
		//temp_speed_motorsF = temp_speed_motorsR;
	}
}


void Func_Pressure_Const_F (unsigned int setPoint )
{
	short			rpm_r, cnt_loop;
	byte			speed_r	= 20.0;
	const float		errMax	= 5;
	float			qw =10, korr,MaxAirFlow,qSetPoint;
	short	                pw, Kmax_Depot;
	float		        qwNew;
	float			deltaP, deltaP_prev;
	float	                pwCalc;
	signed char		deltaPSign;
	unsigned short            Kmin_reg,Kmax_calcDep;
	int                       speedNew = 51,i;
	
	
	rpm_r = (sData.rpm_motors_F[0] + rpm_r_preview[0]) >> 1;
	rpm_r_preview[0] = rpm_r;
	
	// in caso di non rotazione esci dalla funzione
	if(rpm_r < 120)
	return;
	
	
	if(DigitAccessoryOperating(ACC_I2C_FLW1)) //CAPS
	{

		// avendo un dispositivo che legge direttamente la pressione
		// having a device that directly reads the pressure
		// la funzione è molto più semplice
		// the function is much simpler
		pw =  sData.AccI2C[ACC_I2C_FLW1].measure4;
		
		//--------------- correzione limiti di lettura p
		if(pw > 800)
		pw = 800;
		if(pw < 0)
		pw = 10;
		
		//--------------- visualizzo p sul kts
		//sData.measure_pressure = pw;
		Pressure_CAPS = pw;
		//--------------- calcolo qw

		MaxAirFlow = (float)GetMaxAirFlow();
		qSetPoint  = MaxAirFlow * (float) ((float) setPoint / 1000.0);
		speed_r    = (byte)((float)sData.speed_motors_F / 2.55); // step di 0.1V
		
		Kmax_calcDep = KMax * calcDepot();
		Kmin_reg = KMin;
		
		
		for( i=1; i < KRPM_Length;  i++) {
			if(qSetPoint > qmax_rpm[i])
			break;
		}
		
		Kmin_reg = (100 - i + 1); // Kmin_reg : va da 99 a 40


		korr = 0.000243055*qw*qw*0.8;

		Kmin_reg = (100 - i + 1); // Kmin_reg : va da 99 a 40

		if(speed_r >= Kmin_reg)
		qw = CalcQW_by_PW(speed_r, pw-korr);
		else {
			qw = CalcQW_by_PW(Kmin_reg, pw) * 0.95;
		}
		//--------------- visualizzo qw sul kts
		//sData.measure_airflow  = qw;
		Airflow_CAPR = qw;
		
		//inizia la regolazione
		//---------------- fai la regolazione ogni 9 * 1.2 = 10.8 secondi
		if(++cnt_delay1 <= 60)
		return;
		
		//----------------------------------- Nicola 05/05/2016
		// se non c'è stato un aggiornamento dei dati di pressione inutile entrare nel regolatore.
		//         if(sData.AccI2C[ACC_I2C_PCAP].measure1 == 0){
		//           cnt_delay1 = 9;
		//           return;
		//         }
		//
		//         sData.AccI2C[ACC_I2C_PCAP].measure1 = 0;
		//         cnt_delay = 0;
		//------------------------------------------------------------------------
		// Commentato perchè di poca utilità e crea azzeramenti continui sul KTS
		//------------------------------------------------------------------------

		// Impostiamo di default i valori correnti di speed motors
		speedNew = sData.speed_motors_F; // passo il valore precedente
		
		if(pw < (setPoint - 2)) {
			int soglie[4];
			
			soglie[0] = (setPoint >> 1);                // -50.0%
			soglie[1] = setPoint - (setPoint >> 2);     // -25.0%
			soglie[2] = setPoint - (setPoint >> 3);     // -12.5%
			soglie[3] = setPoint - (setPoint * 5)/ 100; //  -5.0%
			
			if(pw < soglie[0]) {
				speedNew += 1;  // + 10%
				cnt_delay1 = -30;  // la correzione è grossa occorre apettare 18 sec., visto l'inerzia dei motori
				}else if (pw < soglie[1]) {
				speedNew += 1;  // + 5%
				cnt_delay1 = -20;  // la correzione abbastanza grossa occorre apettare: 13.2 sec.
				}else if (pw < soglie[2]) {
				speedNew += 1;   // + 2%
				cnt_delay1 = -10;
			}else if (pw <= soglie[3])
			speedNew++;
			
			if(speedNew > 255)
			speedNew = 255;
			
			}else if(pw > (setPoint + 2)){
			int soglie[4];
			
			soglie[0] = setPoint + (setPoint >> 1);     // +50.0%
			soglie[1] = setPoint + (setPoint >> 2);     // +25.0%
			soglie[2] = setPoint + (setPoint >> 3);     // +12.5%
			soglie[3] = setPoint + (setPoint * 5)/ 100; //  +5.0%
			
			if(pw > soglie[0]) {
				speedNew -= 1;  // + 10%
				cnt_delay1 = -30;  // la correzione è grossa occorre apettare 18 sec., visto l'inerzia dei motori
				}else if (pw > soglie[1]) {
				speedNew -= 1;  // + 5%
				cnt_delay1 = -20;  // la correzione abbastanza grossa occorre apettare: 13.2 sec.
				}else if (pw > soglie[2]) {
				speedNew -= 1;
				cnt_delay1 = -10;
			}else if (pw <= soglie[3])
			speedNew--;
			
			if(speedNew < 52)
			speedNew = 52;
		}
		
		temp_speed_motorsF = (speedNew &0x0FF);
		//temp_speed_motorsF = temp_speed_motorsR;
	}
}

//------------------------------------------------------------------------------
// Gestione ventole a portata aria costante:
//    1:  Con accessorio esterno
//    2:  Con calcoli tramite SW
//------------------------------------------------------------------------------
//#define _PRINT_CAF // attiva le stringhe di debug
void Func_AirFlow_Const( unsigned int setPoint )
{
     
    float                   MaxAirFlow;
	float			qSetPoint;
	short			rpm_r;		
	byte			i, speed_r;
	float		        errMax	= 10.0;
	float			qw = 10, pwNew, qw_values[3], korr;
	unsigned short	        Kmax_calcDep, temp_u16, Kmin_reg;
    float                   pw = 10;
	float			delta_P, delta_Q;      
               
  
        // dobbiamo mediare di + gli rpm. 
        for(i=(NUM_AVERAGE_RPM-1); i; i--) 
           rpm_r_preview[i] = rpm_r_preview[i-1];  
       
        rpm_r_preview[0] = sData.rpm_motors_R[0];   
        
        for(i=0, rpm_r = 0; i < NUM_AVERAGE_RPM; i++) 
          rpm_r += rpm_r_preview[i];    
     
        rpm_r /= NUM_AVERAGE_RPM;      
        
         // in caso di non rotazione esci dalla funzione 
        if(rpm_r <= 150)
           return;
        
        if(++cnt_delay <= 6) 
          return;
           
        MaxAirFlow = (float)GetMaxAirFlow();
        qSetPoint  = MaxAirFlow * (float) ((float) setPoint / 1000.0);    
        speed_r    = (byte)((float)sData.speed_motors_R / 2.55); // step di 0.1V            
   
        Kmax_calcDep = KMax * calcDepot();             
        Kmin_reg = KMin;
        
        
        for( i=1; i < KRPM_Length;  i++) {
           if(qSetPoint > qmax_rpm[i])
              break;         
        }  
        
        Kmin_reg = (100 - i + 1); // Kmin_reg : va da 99 a 40
        
//----------------------------------
#ifdef _PRINT_CAF
        Serial_print1("CAF, Rpm: ");
        Serial_print(rpm_r, DEC);
        Serial_print1("  Speed_r: ");
        Serial_print(speed_r, DEC);
        Serial_print1("  PWM_R: ");
        Serial_print(sData.speed_motors_R, DEC);
        Serial_print1("  SepPCaf: ");
        Serial_println(qSetPoint, 1);
#endif
//----------------------------------

#ifdef COMESSA_2388
         // avendo un dispositivo che legge direttamente la pressione 
         // la funzione è molto più semplice
         pw =  sData.AccI2C[ACC_I2C_PCAP].measure1;
         
         if(pw > 800)
              pw = 800;
         if(pw < 0)
             pw = 10;
         
         sData.measure_pressure = pw;
         
         korr = 0.000243055*qw*qw;
         
         if(speed_r >= Kmin_reg) 
           qw = CalcQW_by_PW(speed_r, pw-korr);
         else {
           qw = CalcQW_by_PW(Kmin_reg, pw) * 0.95;
           
           temp_speed_motorsR = ((Kmin_reg + 0.45) * 2.55); 
           temp_speed_motorsF =  temp_speed_motorsR;
           return; 
         }  

#else
	// Calcola punto di lavoro (q, p)
        if(speed_r >= Kmin_reg) { 	 // 40%     
            qw = CalcQW( speed_r, rpm_r ); 
            pw = CalcPW( speed_r, qw );            
        }else{
            
            qw = qmax_rpm[100 - Kmin_reg] * 0.95; 
            pw = CalcPW( speed_r, qw ); 
            
            temp_speed_motorsR = ((Kmin_reg + 0.45) * 2.55); 
            temp_speed_motorsF =  temp_speed_motorsR;
            return;
        }  
	
        if(pw < 3) 
           pw = 3;
          
        if(pw > 750) 
           pw = 750;
#endif

       	      
	// Impostiamo di default i valori correnti di speed motors
	temp_speed_motorsR = sData.speed_motors_R; // passo il valore precedente
	temp_speed_motorsF = temp_speed_motorsR;
      

        // trova le misure mediate da riportare al KTS   
#ifndef COMESSA_2388        
        acc_measure_pressure += (unsigned short)pw;
        sample_measure_pressure++;
    
        if(sample_measure_pressure >= 4) {
          sData.measure_pressure  = (unsigned short)(acc_measure_pressure / sample_measure_pressure);
          acc_measure_pressure    >>= 1;
          sample_measure_pressure >>= 1;
        }   
#endif

        acc_measure_airflow += (unsigned short)qw;
        sample_measure_airflow++;
        
                       
        if (sample_measure_airflow >= 8) {
          sData.measure_airflow   =  acc_measure_airflow / sample_measure_airflow;
          sample_measure_airflow  >>= 1;
          acc_measure_airflow     >>= 1;
        }    
        
        delta_Q  = (float)qSetPoint - (float) qw;   
                
        
        if(cnt_delay < 12) 
          return; 
                  
        cnt_delay = 0; 
        
        // Calcola la Curva di impianto, e la sua distanza dal punto di lavoro.        
        pwNew    = (pw / (long int)(qw * qw)) * ((long int)qSetPoint * qSetPoint);
        delta_P  = pwNew - pw;      
        errMax   = ((MaxAirFlow * 5.0) + 50) / 100;

//----------------------------------
#ifdef _PRINT_CAF
        Serial_print1("Pw: ");
        Serial_print(pw, 1);
        Serial_print1("  Qw: ");
        Serial_print(qw, 1);
        Serial_print1("  ErrMax: ");
        Serial_print(errMax, 1);
        Serial_print1("  dQ: ");
        Serial_print(delta_Q, 1);
        Serial_print1("  Kmax: ");
        Serial_println(Kmax_calcDep, DEC);     
#endif
//-----------------------------------      

        // se il sistema chiede una diversa pressione, e non compromettiamo la portatata, aumentiamo o diminuiamo
	if (((delta_Q >= errMax) || (delta_Q <= -errMax))) //&& (speed_r >= Kmin_reg))  // && (speed_r <= Kmax_calcDep))
	{
             unsigned short  speedNew;
             float calc_new_PWM; 
 
             speedNew = speed_r;  // da 40.0% a 100.0%
             
//----------------------------------
#ifdef _PRINT_CAF
             Serial_print1("SpeedR: ");
             Serial_print(speedNew, DEC);
#endif
//-----------------------------------                                     


             if (delta_Q >= (errMax * 3))      // > 18.0%
               speedNew += 5;     // +5%  
             else if (delta_Q >= (errMax * 2)) // > 12.0%
               speedNew += 2;     // +2%
             else if (delta_Q > 0) 
                speedNew++;       // +1%
             else if (delta_Q < -(errMax * 3))  // < -12.0%
               speedNew -= 5;    // -3%
             else if (delta_Q < -(errMax * 2))  // < -12.0%
                speedNew -= 2;    // -2%
             else 
                speedNew--;       // -1% 

             
             if(speedNew > 100) 
               speedNew = 100;
               
             if(speedNew < Kmin_reg) 
               speedNew = Kmin_reg;
               
               

//----------------------------------
#ifdef _PRINT_CAF
             Serial_print1("  New SpeedR: ");
             Serial_print(speedNew, DEC);
#endif
//-----------------------------------                                     
             calc_new_PWM = ((float)((float)speedNew + 0.45) * 2.55); // da 102 a 255
             if(calc_new_PWM > 255.0)
               calc_new_PWM = 255.0;
               
             temp_speed_motorsR = (byte)calc_new_PWM; // da 102 a 255
	     temp_speed_motorsF = temp_speed_motorsR;    


//----------------------------------
#ifdef _PRINT_CAF   
             Serial_print1("  PWM_R: ");
             Serial_println(temp_speed_motorsR, DEC);
             Serial_println1(" ");
#endif
//----------------------------------

         }
         
                

}


//------------------------------------------------------------------------------
// Gestione ventole con velocitÃ  costante:
// il setPoint Ã¨ espresso in %, da 20.0%  a 100.0% (step di 0.1% = 1)
//------------------------------------------------------------------------------
void Func_SpeedFan_Const(unsigned int setPoint)
{
   unsigned int value_pwm = MINSPEED;
   
   value_pwm = (unsigned int )((float)setPoint * 0.255); //  es: 50.0% = 500 * 0.255 = 127

#ifdef VIEW_MOTORSPEED_DBG  //''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''' x_dbg 
   if((cnt_view_dbg % 4) == 0) { 
       Serial_print1("SetP:");
       Serial_print(setPoint, DEC);
       Serial_print1(" PWM:");
       Serial_print(value_pwm, DEC);
   }
#endif  //''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''' x_dbg          
                               
   temp_speed_motorsR = (byte)value_pwm;
   temp_speed_motorsF = (byte)value_pwm;
}  

//------------------------------------------------------------------------------
// Gestione ventole con velocità proporzionale a temperatura e CO2: - Gestion des ventilateurs avec vitesse proportionnelle à la température et au CO2 :
// il setPoint è espresso in %, da 20.0% (=200)  a 100.0% (=1000) (step di 0.1% = 1) - le setPoint est exprimé en %, de 20,0% (= 200) à 100,0% (= 1000) (pas de 0,1% = 1)
//------------------------------------------------------------------------------
void Temp_CO2_speed(unsigned int setPoint)
{
   
   short soglia_Off_Cooler, soglia_On_Cooler;
   unsigned short SetPointTemp, maxCO2, minCO2, CO2_diff, T_diff,vel_diff, velCO2, velT, NewSetPoint, timer; 
   byte i_rifTemp;
   unsigned int value_pwm, step_pwm, NSP_pwm;
   float mC02, mTemp, vStep;
    
   i_rifTemp    = read_byte_eeprom(ADDR_EEP(idxSetPointT)) & 0x01;
   SetPointTemp = read_word_eeprom(ADDR_EEP(SetPointTemp[i_rifTemp]));   
   soglia_Off_Cooler = SetPointTemp + (signed char)read_byte_eeprom(ADDR_EEP(hister_Temp_Cold[0]));
   soglia_On_Cooler  = SetPointTemp + (signed char)read_byte_eeprom(ADDR_EEP(hister_Temp_Cold[1])); 
   maxCO2 = read_word_eeprom(ADDR_EEP(SetPoint_CO2)); 
   minCO2 = 500;
   
   //verifico il valore di velocità secondo livello di CO2
   CO2_diff = maxCO2 - minCO2;
   vel_diff = 1000 - setPoint;
   mC02 = (float)((float)vel_diff / (float)CO2_diff);
   
   if (sData.measure_CO2_max >= maxCO2) //sono maggiore del set point devo andare al massimo
   {
     velCO2 = 1000;
   }else if ((sData.measure_CO2_max > minCO2) && (sData.measure_CO2_max < maxCO2) ) //sono maggiore del set point minimo e minore del massimo: devo andare in modo proporzionale
   {
     velCO2 = mC02 * (sData.measure_CO2_max - minCO2) + setPoint;
   }
    else //sono minore del valore minimo di CO2 nt devo andare al massimo
    velCO2 = setPoint;
     
   
    //verifico il valore di velocità secondo livello di temperatura
    T_diff = soglia_On_Cooler - soglia_Off_Cooler;  //Sono in freddo mi accendo sopra il setpoint e mi spengo sotto, cosi scrivendo ho una differenza sempre positiva
    mTemp =(float)((float)vel_diff / (float)T_diff);
   
   if (sData.measure_Temp[I_PROBE_RETURN] >= soglia_On_Cooler) //sono maggiore del set point devo andare al massimo
   {
     velT = 1000;
   }else if ((sData.measure_Temp[I_PROBE_RETURN] > soglia_Off_Cooler) && (sData.measure_Temp[I_PROBE_RETURN] < soglia_On_Cooler) ) //sono maggiore del set point minimo e minore del massimo: devo andare in modo proporzionale
   {
     velT = mTemp*(sData.measure_Temp[I_PROBE_RETURN] - soglia_Off_Cooler)+setPoint;
   }
    else //sono minore del valore minimo di CO2 nt devo andare al massimo
    velT = setPoint;
 
     
   if (output_on) //considero il maggiore tra CO2 e Temperatura se attivo il FreeContact Sum/Win
   {
    if (velCO2 >= velT)//considero il maggiore tra CO2 e Temperatura
     NewSetPoint =  velCO2;
     else
     NewSetPoint =  velT;     
 
   }else // altrimenti solo la CO2 (siamo in inverno)
   {
    NewSetPoint =  velCO2;  
   }
   
 
   
   //Creo il contatore che rispetti la constante di tempo 5 min dal Setpoint sul KTS alla MASSIMA velocità
   // 1.2 sec al ciclo => 5 min = 300 sec. = 250 cicli
      
   vStep = (float)((float)vel_diff/(float)TIMER_MAX);
   
   if (vStep < 4) //step_pwm sarebbe sempre 0 devo fare dei cicli a vuoto
   {
    
     if ( conta_step > 4)
     {
     conta_step = conta_step -4; //torno sotto al quattro mantenendo il valore parziale
     step_pwm = 1; //compio lo step di regolazione
     conta_step = conta_step+vStep; //aumento conta step del valore necessario
     }
     else
     {
     conta_step = conta_step+vStep;
     step_pwm = 0;
     }
   }
   else
   {
   step_pwm =  (unsigned int )((float)vStep * 0.255); //  es: 50.0% = 500 * 0.255 = 127  
   }
   

   NSP_pwm = (unsigned int )((float)NewSetPoint * 0.255); //  es: 50.0% = 500 * 0.255 = 127       
   
   
      
      
   if (NSP_pwm  < MINSPEED) //controllo sempre di non scendere sotto il livello minimo
   NSP_pwm = MINSPEED;
   if (NSP_pwm  > 255) //controllo sempre di non salire sopra il livello massimo
   NSP_pwm = 255;

   if(sData.speed_motors_R < MINSPEED)
     sData.speed_motors_R = MINSPEED;
   
   if (NSP_pwm < sData.speed_motors_R)
   {
     if((sData.speed_motors_R - step_pwm) < MINSPEED)
     {
        sData.speed_motors_R = MINSPEED;
     }
     else
     {
        sData.speed_motors_R = sData.speed_motors_R - step_pwm;
     }

   }
   if (NSP_pwm > sData.speed_motors_R)   
   {
     if((sData.speed_motors_R + step_pwm) > 255)
     {
        sData.speed_motors_R = 255;
     }
     else
     {
        sData.speed_motors_R = sData.speed_motors_R + step_pwm;
     }     
   }
    sData.speed_motors_F = sData.speed_motors_R; 
    sData.increase_speed_RH_CO2 = (byte) ((float)sData.speed_motors_R * (float)39.3 / (float)setPoint );
    
    
//    
//    Serial_println1("* * * * * * * * * * * * * * * * * * *");
//    Serial_print1("velCO2:");
//    Serial_print(velCO2, DEC);
//    Serial_print1(" velT:");
//    Serial_print(velT, DEC);
//    Serial_print1(" soglia_Off_Cooler:");
//    Serial_print(soglia_Off_Cooler, DEC);
//    Serial_print1(" vel_diff:");
//    Serial_print(vel_diff, DEC);
//    Serial_print1(" CO2_diff:");
//    Serial_print(CO2_diff, DEC);
//    Serial_print1(" measureCO2:");
//    Serial_print(sData.measure_CO2_max, DEC);
//    Serial_print1(" motors_R:");
//    Serial_print( sData.speed_motors_R, DEC);  
//    Serial_print1(" NewSetPoint:");
//    Serial_println(NewSetPoint, DEC);
//    Serial_println1("* * * * * * * * * * * * * * * * * * *");
}

#ifdef _MAPPING_FUNC
//-----------------------------------------------------------------
// stampa la mappatura SPEED e rpm.
//------------------------------------------------------------------
static float percent_speed = 40.0;
static byte  print_startup   = 1;
static byte  test_runnig     = 0;
static short sec_startup     = 0;

void debug_mapping_motors(void)
{
   short  rpm_r;	
   float  speed_r;
   byte i, c, nb;
   byte buff[32];
   

   if(print_startup) {
     print_startup = 0;
     Serial_println1(" ");
     Serial_println1("* * * * * * * * * * * * * * * * * * *");
     Serial_println1("      *  MAPPING MOTORS   *");
     Serial_println1(" Press KEY 1: start Test in 1 min.");
     Serial_println1(" Press KEY 2: start Test in 2 min.");
     Serial_println1("* * * * * * * * * * * * * * * * * * *");
     // si parte dal 100%
     percent_speed = 100.0; 
     speed_r = percent_speed * 2.55;       
     temp_speed_motorsR = speed_r;
     temp_speed_motorsF = temp_speed_motorsR;  
   }  
   
   nb = Serial_available();
   
   if(nb) {
     for(i=0; i < nb; i++)
       buff[i] = Serial_read();   
        
     if(buff[0] == '1') 
     {
       test_runnig = 1;
       sec_startup = 59; 
       cnt_delay = 1;
       Serial_println1(" ");
       Serial_println1("KEY 1, speed 100%, wait 1 min.");
 
       percent_speed = 100.0; 
       speed_r = percent_speed * 2.55;       
       temp_speed_motorsR = speed_r;
       temp_speed_motorsF = temp_speed_motorsR;  
     }else if (buff[0] == '2') {
       
       test_runnig = 1;
       
       sec_startup = 120; 
       cnt_delay = 1;
       Serial_println1(" ");
       Serial_println1("KEY 2, speed 100%, wait 2 min.");
    
       percent_speed = 100.0; 
       speed_r = percent_speed * 2.55;       
       temp_speed_motorsR = speed_r;
       temp_speed_motorsF = temp_speed_motorsR;        
      } 
     
   }  

   if(!test_runnig)
     return;
  
   if(sec_startup) {
     sec_startup--;
     if(sec_startup == 90)
       Serial_println1("wait 90 sec.");
     else if(sec_startup == 60)
       Serial_println1("wait 60 sec");  
     else if(sec_startup == 30)
       Serial_println1("wait 30 sec");    
     else if(sec_startup == 1){
       Serial_println1(" ");
       Serial_println1("Speed %;  rpm_r  ;  rmp_f");
     }  
     return;
   }     
         
             
   cnt_delay++;
   
   if((cnt_delay == 32) || (cnt_delay == 48)){
      if(percent_speed >= 100)
        Serial_print1("  ");
      else
        Serial_print1("   ");
        
      Serial_print(percent_speed, 1);
      Serial_print1(" ; ");
      Serial_print(sData.rpm_motors_R[0], DEC); 
      Serial_print1(" ; ");
      Serial_println1(sData.rpm_motors_F[0], DEC);         
   }else if (cnt_delay >= 52) {       
      cnt_delay = 1;
    
      // incrementiamo la velocita di 0,5 Volt
      percent_speed -= 5;    
      if(percent_speed < 40) {
        Serial_println1(" ");
        Serial_println1("End Test, press Keys '1' or '2' for restart Test.");
        test_runnig = 0;
        percent_speed = 40;
        
      }  
               
      speed_r = percent_speed * 2.55;
     
      temp_speed_motorsR = speed_r;
      temp_speed_motorsF = temp_speed_motorsR; 
           
   }

       
}  
#endif

//------------------------------------------------------------------------------
// Calcola la velocitÃ  dei due motori in base agli stati degli accessori, 
// ed alle prioritÃ  delle impostazione di funzionamento. 
// La funzione gira nello sheduler ogni 1,2 sec.
// 
//
//        FUNZIONAMENTO       |       funzioni Disabilitate 
// ---------------------------+-------------------------------------------------
//     DeFROST                   CAF | CAP | IMBALANCE | NO_INP_CTRL
//     VOC                           | CAP | IMBALANCE | NO_INP_CTRL   
//     RH                            | CAP | IMBALANCE | NO_INP_CTRL    
//     Co2                           | CAP | IMBALANCE | NO_INP_CTRL  
//     WEEKLY:                                         | NO_INP_CTRL 
//     CAP  (pressione cost.)                IMBALANCE 
//     CAF  (portata  cost.)                 IMBALANCE 
//------------------------------------------------------------------------------

int motor_speed() 
{  
   unsigned int idx , spd_1, spd_2, diff, addr1, addr2, val_16;
   byte PwrOff, unit_standby = 0, status_inputs,  sensorAir_op = 0, minSpeedMotors = MINSPEED, temp_08, buff_temp[4], nNtc = 2, status_output1, status_output2, setpoint_airflow_co2;
   char temp;
   unsigned short enab_func, setPoint =0, temp16, setPoint1 =0;
   short sgl_qAir;
   float sbil_fans = 0;
    

#ifdef _MAPPING_FUNC
   debug_mapping_motors();
#else
    
   if(CkAlarm(ALM_EB_CTRL_FAN)) 
      return 1;
   
   //------------------------------------------------
   // Verifico la presenza di allarmi acqua nel phwd
   // e nel caso tengo fermi i motori
   // ALM_PEH_NTC_KO,    ALM_PEH_TW_DANGER, ALM_PEH_TW_OVR,    ALM_PEH_ELECT,     ALM_PEH_TA_OVR,    ALM_PEH_SIZE,     ALM_PEHD_PRESS,     ALM_PEH_LINK,   // byte 02
   //------------------------------------------------
   if((sData.AccI2C[ACC_I2C_PREHEATER].sts & STS_ACC_ELECT)==0)
     {
       if(CkAlarm(ALM_PEH_TW_DANGER) || CkAlarm(ALM_PEH_TW_OVR) || CkAlarm(ALM_PEH_NTC_KO) || CkAlarm(ALM_PEH_ELECT))
        {
          
         temp_speed_motorsR = 0;
         temp_speed_motorsF = 0;
         
         sData.speed_motors_R = temp_speed_motorsR;
         sData.speed_motors_F = temp_speed_motorsF;
         
         return 1;
        }
      }
  
   //------------------------------------------------
   // 0: Il Modbus prende il controllo dei motori in maniera separata
   //
   //------------------------------------------------
    // leggiamo le funzioni abilitate
   enab_func = read_word_eeprom(ADDR_EEP(Enab_Fuction)); 

 
   if ((enab_func & (1 << ENAB_MBF)) && (AccessoryPresent_EepList(ACC_EEP_MBUS)) )
     {
     sData.status_unit |= (1 << POS_BIT_UNIT_RUN); 
     temp_speed_motorsR = (unsigned int )((float)read_byte_eeprom(ADDR_EEP(Set_MBF_return)) * 2.55);
     temp_speed_motorsF = (unsigned int )((float)read_byte_eeprom(ADDR_EEP(Set_MBF_fresh)) * 2.55); 
     sData.speed_motors_R = temp_speed_motorsR;
     sData.speed_motors_F = temp_speed_motorsF;
     return 1;
     }
     
	if ( AccessoryPresent_EepList(ACC_EEP_P1CO2) || AccessoryPresent_EepList(ACC_EEP_P2CO2)) // check if unit is in standby mode for more than 2 hours
	 {
		 if (read_byte_eeprom(ADDR_EEP(Set_Power_ON)) == 0 ) // check if unit is in standby mode
		 {
			 if ( Clean_Event_Counter_2_hours >= 7200000 ) // unit is in standby for more than 2 hours
			 {
				 Clean_Event_Counter_2_hours = 0;
				 if (( Index_CO2_1 != 0) || ( Index_CO2_2 != 0)) // last clean event in 24 hours
				 {
					 if ( Clean_Event_Flag1 || Clean_Event_Flag2)
					 {
						 Clean_Event_Flag1 = false;
						 Clean_Event_Flag2 = false;
						 sData.status_unit |= (1 << POS_BIT_CLEANEVENT_CHECK); // activate clean event , KTS go to ON mode
					 }
				 }
				 
			 }
		 }
	 }
	 
	 if ((enab_func & (1 << ENAB_CLEAN_EVENT)) && (AccessoryPresent_EepList(ACC_EEP_P1CO2) || AccessoryPresent_EepList(ACC_EEP_P2CO2)))
	 {
		 if ( Clean_Event_Counter <= 1800000) // clean event counter 30 min
		 {
			 sData.status_unit |= (1 << POS_BIT_CLEANEVENT_CHECK);
			 temp_speed_motorsR = (unsigned int )(80 * 2.55);
			 temp_speed_motorsF = (unsigned int )(80 * 2.55);
			 sData.speed_motors_R = temp_speed_motorsR;
			 sData.speed_motors_F = temp_speed_motorsF;
			 Clean_Event_Flag1 = false;
			 Clean_Event_Flag1 = false;
			 Index_CO2_1 = 0;
			 Index_CO2_2 = 0;
			 CO2_1_Average_Counter = 0;
			 CO2_2_Average_Counter = 0;
			 return 1;
		 }
		 else // 30 min elapsed
		 {
			 enab_func &=  ~(1 << ENAB_CLEAN_EVENT);
			 Clean_Event_Counter = 0;
			 temp_08 = read_byte_eeprom(ADDR_EEP(cntUpdate_info));
			 temp_08++;
			 
			 write_word_eeprom(ADDR_EEP(Enab_Fuction), enab_func);
			 write_byte_eeprom(ADDR_EEP(cntUpdate_info), temp_08);
		 }
	 }
	 else
	 {
		sData.status_unit &= ~(1 << POS_BIT_CLEANEVENT_CHECK);
		Clean_Event_Counter = 0;
	 }
	 
   // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
   // 1: Verifichiamo le impostazioni degli input e il relativo comando.
   //      0x84 = Input Config as Allarme Incendio & Extract OFF, Supply MAX.  // Dalla 2.15 
   //      0x83 = Input Config as Allarme Incendio & Extract Max, Supply OFF.  // Dalla 2.15   
   //      0x82 = Input Config as Allarme Incendio & Extract & Supplay Max.    // Dalla 2.15
   //      0x81 = Input Config as Reg_AirFlow/ Allarme incedio & Unit Stop. 
   //      0x80 = Input Config as Reg_AirFlow & Unit RUN
   //      0x11 = Input Config as (UNIT_ON/OFF | Allarme incendio) & Unit Stop 
   //      0x10 = Input Config as UNIT_ON/OFF & Unit RUN   
   //      0x00 = Input NO Config 
   // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
   status_inputs = ctrl_comand_inputs();
   
   
   status_output1 = read_byte_eeprom(ADDR_EEP(Set_Output[0]));
   status_output2 = read_byte_eeprom(ADDR_EEP(Set_Output[0]));
    
   
#ifdef VIEW_MOTORSPEED_DBG //''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''' x_dbg       
   cnt_view_dbg++; 
   if((cnt_view_dbg % 4) == 0) { 
     Serial_println1(" ");  
     Serial_print1("Ctrl Input: ");
     if(status_inputs)
        Serial_println1("Standby unit");
     else
        Serial_println1("- - -");   
   } 
   
#endif //'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''' x_dbg     

   if((status_inputs == 0x81) || (status_inputs == 0x11)) {
     // almeno un input mi dice che devo fermarmi 
     unit_standby = 0x01; 
     
   }else {
     // Se non sono in regolazione con un input non considero lo stato in eep del ON/OFF ed il weekly
     if ((status_inputs & 0x80) == 0) {
         PwrOff = !read_byte_eeprom(ADDR_EEP(Set_Power_ON));   //  1 byte:   1= SET UNIT IDLE (power_on), 0 = SET UNIT STANDBY (power_off) 
         
         // se e' impostato il weeklyP verifichiamo lo stato. E' chiaro che se l'input dice di stare in OFF, 
         // l'unita' si deve portare in OFF, a prescindere dallo stato in cui lavora il weekly (RUN / OFF)
         if (sData.status_weekly & MSK_BIT_WEEKLY_ENAB)
         {
	         if ((sData.status_weekly & MSK_BIT_WEEKLY_RUN) == 0) {
		         // STANDBY da weekly: il PIR potrà forzare l'accensione se attivo
		         unit_standby |= 0x01;
		         // NON forziamo più PwrOff = 1, lo lasciamo come da EEPROM
	         }
	         else {
		         // Se weekly RUN e PwrOff ancora a 1, lo forziamo in ON
		         if (PwrOff) {
			         write_byte_eeprom(ADDR_EEP(Set_Power_ON), 0x01);
			         PwrOff = 0;
		         }
	         }
         }         
         
         unit_standby = PwrOff; // 1 = StandBy, 0= RUN
      
#ifdef VIEW_MOTORSPEED_DBG //''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''' x_dbg     
         if((cnt_view_dbg % 4) == 0) 
         {   
            Serial_print1("Weekly: ");
            if (sData.status_weekly & MSK_BIT_WEEKLY_ENAB) {
               if(sData.status_weekly & MSK_BIT_WEEKLY_RUN)
                 Serial_println1("RUN");
               else 
                 Serial_println1("Stanby");
            }else  {
                 Serial_print(enab_func, HEX);
                 Serial_println1(", Disable");  
            }     
           
            Serial_print1("Pwr: ");
            if (read_byte_eeprom(ADDR_EEP(Set_Power_ON)))
              Serial_println1("ON");
            else  
              Serial_println1("OFF");
         }   
#endif  //'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''' x_dbg                                         
         
     }else 
        unit_standby = 0;       
   }  

#if 0
   // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
   // 2: Se vi e' un guasto di un motore, fai 3 tentativi dopo di che metti in standy la macchina.
   //    StatusMotors#: 7.-- | 6.AlarmM3 | 5.AlarmM2 | 4.AlarmM1 | 3.-- | 2.PowerON_M3 | 1.PowerON_M2 | 0.PowerON_M1
   // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -  
   if ((sData.StatusMotorsR & 0xF0) || (sData.StatusMotorsF & 0xF0)) { 
      // facciamo 1 tentativi, dopo di che spegniamo l'unita' e attendiamo con pazienza il tecnico. 
      cnt_restart_motors++; 
      if (!num_tentativi_ripartenza && (cnt_restart_motors  < 30))
         cnt_restart_motors = 32; // alla prima segnalazione sto in on per 40 sec. max
        
      if(cnt_restart_motors < 70)  // 70 sec
         PowerMode(POWER_ON);     
      else if (cnt_restart_motors <= 145)    // 75 secondi Stanby          
      {  
         unit_standby = 0x01;
         if(cnt_restart_motors >= 144)
           cnt_restart_motors = 0;
           
         if (++num_tentativi_ripartenza >= 3) {
           PowerMode(POWER_SAVE);  

           write_byte_eeprom(ADDR_EEP(Set_Power_ON), 0x00); // off
           temp_08 = read_byte_eeprom(ADDR_EEP(cntUpdate_SettingPar)); 
           temp_08++;
           write_byte_eeprom(ADDR_EEP(cntUpdate_SettingPar),  temp_08);  
           
           num_tentativi_ripartenza = 1;
           if(DigitAccessoryOperating(ACC_I2C_PREHEATER))
             TAG_DigitAccessoryOff(ACC_I2C_PREHEATER); 
           
           if(DigitAccessoryOperating(ACC_I2C_HEATER))  
             TAG_DigitAccessoryOff(ACC_I2C_HEATER); 
             
           if(DigitAccessoryOperating(ACC_I2C_COOLER))  
             TAG_DigitAccessoryOff(ACC_I2C_COOLER); 
           return 1;
         }
      }   
   }else { 
      cnt_restart_motors = 0;
      num_tentativi_ripartenza = 0;
   }   
#endif
         
   // se dopo i primi controlli risulta UNIT_OFF, tieni spenti i motori
   if(unit_standby) {
     switchesToStandby (); 
#ifdef VIEW_MOTORSPEED_DBG  //''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''' x_dbg   
     if((cnt_view_dbg % 4) == 0)
         Serial_println1("Standby Unit"); 
#endif     
     return 1;
   }
  
   
   //Se eravamo in stanby,  prima di mandare il comando ai motori accendo l'elettronica, pilotando il relay di potenza
   if(PowerMotors == POWER_SAVE) {
     PowerMode(POWER_ON);     
     return 1;
   }  
   
   count_switch_off = 0;
   //digitalWrite(pPowerMotors, LOW);
   ioport_set_pin_level(pPowerMotors, IOPORT_PIN_LEVEL_LOW);
   sData.status_unit |= (1 << POS_BIT_UNIT_RUN); 
   
   // In fase di startup, porto i motori oltre il 33% per 30 sec.
   // Questo mi permette di evitare la NON partenza di qualche motore se 
   // lavorassimo al 20%/21%. Una volta partiti, non ci sono + problemi.   
   if(count_switch_on++ < 25)  
      minSpeedMotors = MINSPEED_STARTUP;                        
   else {  
      minSpeedMotors = MINSPEED;
      count_switch_on = 26;
   }   
   
   // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -  
   // 3. Durante la chiusure del BYPASS l'aria deve girare al minimo
   //    per evitare assorbimenti anomali, e quindi conclusioni errate
   //    sul fine corsa.
   // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -   
   if (sData.status_unit &  (1 << POS_BIT_BYPASS_RUN)) {
       sData.speed_motors_R = minSpeedMotors;
       sData.speed_motors_F = minSpeedMotors;
       return 1;
   }    


   // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -  
   // 3.1 Durante la calibrazione o la verifica del dpp demandiamo il
   //     controllo a un'altra funzione
   //    
   // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -   
 
    if (sData.status_unit &  (1 << POS_BIT_DPP_CHECK)) {
     
       return 1;
   }    
 
 
   // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -  
   // 4. Se la regolazione non è fatta da un input, andiamo a ragionare sui vari 
   //    settaggi in eeprom (weekly, CAP, etc..).
   // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
         
   if ((status_inputs & 0x80) == 0)
   {     
     if (sData.status_weekly  & MSK_BIT_WEEKLY_ENAB)
     { 
         // Funzione weekly abilitata:     
       if (sData.status_weekly & MSK_BIT_WEEKLY_RUN) {
          idx = (sData.status_weekly >> POS_STEP_SPEED) & 0x03;  // bit[7,6]: 1,1=----/ 1,0=SPEED_3/ 0,1=SPEED_2/ 0,0=SPEED_1
                   
          if(enab_func & (1<< ENAB_CAP)) {
            setPoint = read_word_eeprom(ADDR_EEP(Set_StepMotors_CAP[3]));
			setPoint1 = read_word_eeprom(ADDR_EEP(Set_StepMotors_CAP[1]));
			if(DigitAccessoryOperating(ACC_I2C_PCAP)) // cap
			{
				Func_Pressure_Const(setPoint); // pressione costante
			}
			else // caps , capr
			{
				Func_Pressure_Const_F(setPoint); // pressione costante
				Func_Pressure_Const_R(setPoint1); // pressione costante
			}
          }else {
            setPoint = read_word_eeprom(ADDR_EEP(Set_StepMotorsCFS_CAF[idx]));
            if (enab_func & (1 << ENAB_CAF)) 
              Func_AirFlow_Const(setPoint);   // portata costante
            else  
              Func_SpeedFan_Const(setPoint);  // Velocita'  costante     
          }  
       }
     }else {
        // NON E' ATTIVO IL WEEKLY, VERIFICHIAMO LE IMPOSTAZIONE MANUALI
        idx = read_byte_eeprom(ADDR_EEP(sel_idxStepMotors));
        
        //  1 byte: Selettore dell'indice degli array Set_StepMotors[4]: 0, 1, 2, 3=Steepless 
        if(idx > 3)
          idx = 3;
        
        
        if(enab_func & (1<< ENAB_CAP)) {

#ifdef VIEW_MOTORSPEED_DBG  //''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''' x_dbg 
            if((cnt_view_dbg % 4) == 0) 
               Serial_print1("CAP func: ");
#endif   //'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''' x_dbg            
                    
             setPoint = read_word_eeprom(ADDR_EEP(Set_StepMotors_CAP[3]));
             setPoint1 = read_word_eeprom(ADDR_EEP(Set_StepMotors_CAP[1]));
			 if(DigitAccessoryOperating(ACC_I2C_PCAP)) // cap
			 {
				 Func_Pressure_Const(setPoint); // pressione costante
			 }
			 else // caps, capr
			 {
				 Func_Pressure_Const_F(setPoint); // pressione costante
				 Func_Pressure_Const_R(setPoint1); // pressione costante
			 }
            // verifico che nessun sensore mi obblighi una regolazione diversa, e quindi torno a lavorare con il CAF rispetto al setpoint
        }else if ((enab_func & (1 << ENAB_CAF)) &&  !((sData.status_unit & (1 << POS_BIT_MAX_VOC)) || (sData.status_unit & (1 << POS_BIT_MAX_CO2)) || (sData.status_unit & (1 << POS_BIT_MAX_RH))) ){ 
          

#ifdef VIEW_MOTORSPEED_DBG  //''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''' x_dbg 
            if((cnt_view_dbg % 4) == 0) 
               Serial_print1("CAF func: ");
#endif   //'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''' x_dbg            
                           
            setPoint = read_word_eeprom(ADDR_EEP(Set_StepMotorsCFS_CAF[idx]));
            Func_AirFlow_Const(setPoint);   // portata costante
        }else { // ENAB_CSF
 
#ifdef VIEW_MOTORSPEED_DBG  //''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''' x_dbg 
            if((cnt_view_dbg % 4) == 0)   
                Serial_print1("CFS func: ");
#endif   //'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''' x_dbg                                                 
            setPoint = read_word_eeprom(ADDR_EEP(Set_StepMotorsCFS_CAF[idx]));
            Func_SpeedFan_Const(setPoint);  // VelocitÃ  costante         
        }  
#ifdef VIEW_MOTORSPEED_DBG  //''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''' x_dbg 
        if((cnt_view_dbg % 4) == 0)   
            Serial_println1(" ");
#endif   //'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''' x_dbg                                                     
     }   
     
     // = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =
     // Se non è attiva una della attività legate all'Incedio, procedi con il 
     // defrost, sensori CO2, RH e VOC, etc... 
     // = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =  

     // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
     // 5. Controlla la condizione di DeFROST, se la funzione Ã¨ abilitata
     // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
     if(enab_func & (1 << ENAB_DEFROST))
        deFrostFunc();
     
     // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
     // 6. Se non sono in deFrost Attivo o CAP, verifico:
     //      1: status Sensors VOC  
     //      2: status Sensors RH  
     //      3: status Sensors Co2 
     //      4: Boost ON
     //      5: Imbalance ON   
     // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
     probeQAirActive();
     
     if (((sData.status_unit & (1 << POS_BIT_DEFROST_OPERATING))== 0) && !(enab_func & (1<< ENAB_CAP)))
     {   
        sensorAir_op = 0;
        spd_1 = temp_speed_motorsR; 
        
        if(spd_1 > 255)
           spd_1 = 255;
             
        //--- (1) Controlla il valore di VOC se ho almeno 1 sensore attivo (analogico/digitale) ---         
        if (DigitAccessoryOperating(ACC_I2C_VOC) && !(sData.status_test & MSK_TEST_ENABLE))  
        {  
           setPoint = read_word_eeprom(ADDR_EEP(SetPoint_VOC));
           
           // Setpoint: min:8 PPM, max:90 PPM  
           if ((setPoint < 7) ||  (setPoint > 90)) {
             write_word_eeprom(ADDR_EEP(SetPoint_VOC), 10);
             setPoint = 15;
           }                    
           if(setPoint < 10) {
             sgl_qAir = (setPoint - 4);  
             temp16   = (256 - spd_1)/4;
           }else if (setPoint < 15) { 
             sgl_qAir = (setPoint - 7);  
             temp16   = (256 - spd_1)/7;
           }else {
             sgl_qAir = (setPoint - 10); 
             temp16   = (256 - spd_1)/10;
           }  
  
  #ifdef VIEW_MOTORSPEED_DBG //''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''' x_dbg       
           if((cnt_view_dbg % 4) == 0){  
               Serial_println1(" ");
               Serial_print1("VOC: ");
               Serial_print(sData.measure_VOC_max, DEC);    
               Serial_print1(" Sgl_Air: ");
               Serial_print(sgl_qAir, DEC); 
           }   
  #endif  //'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''' x_dbg          
           
           if(sData.measure_VOC_max > setPoint) {
             sensorAir_op = 1;
             sData.status_unit |= (1 << POS_BIT_MAX_VOC);
             spd_1 = 255;
  #ifdef VIEW_MOTORSPEED_DBG  //''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''' x_dbg 
             if((cnt_view_dbg % 4) == 0)
               Serial_println1(" VOC MAX!");
  #endif   //'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''' x_dbg                                                         
           }else if(sData.measure_VOC_max > sgl_qAir) { 
             sensorAir_op = 1; 
             sData.status_unit |= (1 << POS_BIT_MAX_VOC);
             
             diff = (sData.measure_VOC_max - sgl_qAir) * temp16;      
             spd_2 = temp_speed_motorsR  + diff; 
             
             if(spd_2 > 255)
               spd_2 = 255;
  
             if(spd_2 > spd_1) 
               spd_1 = spd_2;  
             
  #ifdef VIEW_MOTORSPEED_DBG  //''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''' x_dbg 
             if((cnt_view_dbg % 4) == 0){ 
               Serial_print1(" Increase: ");
               Serial_println(diff, DEC);   
             }     
  #endif   //'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''' x_dbg           
           }else 
             sData.status_unit &= ~(1 << POS_BIT_MAX_VOC);
        }else
           sData.status_unit &= ~(1 << POS_BIT_MAX_VOC);
      
        //--- (2) Controlla il valore di RH se ho almeno 1 sensore attivo (analogico/digitale) ---      
        if ((DigitAccessoryOperating(ACC_I2C_RH_1) || DigitAccessoryOperating(ACC_I2C_RH_2)) && !(sData.status_test & MSK_TEST_ENABLE))  
        {        
           setPoint = read_byte_eeprom(ADDR_EEP(SetPoint_RH));
           // Setpoint min:20% max:99%
           if ((setPoint < 20) ||  (setPoint > 99)) {
             write_byte_eeprom(ADDR_EEP(SetPoint_RH), 50);
             setPoint = 50;
           }  
           
           if(setPoint < 50) {
             sgl_qAir = (setPoint - 10); 
             temp16   = (256 - spd_1) / 10; 
           }else {
             sgl_qAir = (setPoint - 12); 
             temp16   = (256 - spd_1) / 12; 
           } 
           
  #ifdef VIEW_MOTORSPEED_DBG  //''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''' x_dbg   
           if((cnt_view_dbg % 4) == 0){ 
               Serial_println1(" ");
               Serial_print1("RH: ");
               Serial_print(sData.measure_RH_max, DEC);
               Serial_print1(" Sgl_Air: ");
               Serial_println(sgl_qAir, DEC);
           }
  #endif  //''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''' x_dbg            
             
           if(sData.measure_RH_max >= setPoint) {
             sensorAir_op = 1;
             spd_1 = 255;        
  #ifdef VIEW_MOTORSPEED_DBG  //''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''' x_dbg 
             if((cnt_view_dbg % 4) == 0) 
                Serial_print1(" RH MAX!");
  #endif  //''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''' x_dbg                                                      
             sData.status_unit |= (1 << POS_BIT_MAX_RH);
           }else if(sData.measure_RH_max > sgl_qAir) {
             sensorAir_op = 1;
             sData.status_unit |= (1 << POS_BIT_MAX_RH);
             diff = (sData.measure_RH_max - sgl_qAir) * temp16;
         
             spd_2 = temp_speed_motorsR  + diff; 
                      
             if(spd_2 > 255)
               spd_2 = 255;
  
             if(spd_2 > spd_1) 
               spd_1 = spd_2;      
               
  #ifdef VIEW_MOTORSPEED_DBG  //''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''' x_dbg 
             if((cnt_view_dbg % 4) == 0){ 
               Serial_print1(" Increase: ");
               Serial_print(diff, DEC);
             }  
  #endif  //'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''' x_dbg                                                        
           }else
             sData.status_unit &= ~(1 << POS_BIT_MAX_RH);        
       }else
          sData.status_unit &= ~(1 << POS_BIT_MAX_RH);   
       
       //--- (3) Controlla il valore di Co2 se ho almeno 1 sensore attivo (analogico/digitale) ---  Vérifier la valeur Co2 si j'ai au moins 1 capteur actif (analogique/numérique)
       if ((DigitAccessoryOperating(ACC_I2C_CO2_1) || DigitAccessoryOperating(ACC_I2C_CO2_2)) && !(sData.status_test & MSK_TEST_ENABLE))   
       {       

          // verifico se ho attivato la modalità sum/win oltre alla presenza dela CO2 - je vérifie si j'ai activé le mode somme/gagnant en plus de la présence de CO2
          // Nel caso devo considerare una funzione diversa che decida la velocità del - Au cas où je devrais envisager une fonction différente qui décide de la vitesse de la
          // motore anche in base al setpoint di temperatura richiesto (pompa di calore a valle) - moteur également en fonction de la consigne de température souhaitée (pompe à chaleur en aval)
         //  if    ((status_output1 &0x07) == 5 || (status_output2 &0x07) == 5)
		 if(0)	 //sempre falso, usata solo per ECS al PSI Forum, forziamo ELSE
         {
           
           Temp_CO2_speed(setPoint);
           return 1; 
         }
         else
         {
         
         
          setPoint = read_word_eeprom(ADDR_EEP(SetPoint_CO2));
          
         // setpoint da:600 PPM a 1500:PPM
         if ((setPoint < 590) ||  (setPoint > 1500)) {
	         write_word_eeprom(ADDR_EEP(SetPoint_CO2), 750);
	         setPoint = 600;
         }
         
         if(setPoint < 900)
         sgl_qAir = (setPoint - 220);
         else
         sgl_qAir = (setPoint - 260);
         
         if( sgl_qAir <= 600)
			sgl_qAir = 600; 
            
  #ifdef VIEW_MOTORSPEED_DBG  //''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''' x_dbg  
          if((cnt_view_dbg % 4) == 0){
              Serial_println1(" ");
              Serial_print1("CO2: ");
              Serial_print(sData.measure_CO2_max, DEC);
              Serial_print1(" Sgl_Air: ");
              Serial_println(sgl_qAir, DEC);
           } 
  #endif  //''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''' x_dbg  
  
          if(sData.measure_CO2_max >= setPoint) {
             sensorAir_op = 1;
             //spd_1 = 255;
             setpoint_airflow_co2 = read_byte_eeprom(ADDR_EEP(SetPoint_Airflow_CO2));
             spd_1 = ( 51 * setpoint_airflow_co2 ) / 20;
             sData.status_unit |= (1 << POS_BIT_MAX_CO2);
             if(spd_1 > 255)
				spd_1 = 255;
             
  #ifdef VIEW_MOTORSPEED_DBG  //''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''' x_dbg 
             if((cnt_view_dbg % 4) == 0)
               Serial_println1(" CO2 MAX!");
  #endif  //''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''' x_dbg            
                                                
          }else if(sData.measure_CO2_max > sgl_qAir) {
             sensorAir_op = 1;
             sData.status_unit |= (1 << POS_BIT_MAX_CO2);
             
             spd_1 = temp_speed_motorsR;
             
             setpoint_airflow_co2 = read_byte_eeprom(ADDR_EEP(SetPoint_Airflow_CO2));
             setpoint_airflow_co2 = ( 51 * setpoint_airflow_co2 ) / 20;
             
             if(setpoint_airflow_co2 > 255)
				setpoint_airflow_co2 = 255;
             
             diff = ((unsigned int)(setpoint_airflow_co2 - spd_1) * (sData.measure_CO2_max - sgl_qAir))/ (setPoint - sgl_qAir);
             
             spd_1 = temp_speed_motorsR  + diff;
             
             temp_speed_motorsR = spd_1;
             temp_speed_motorsF = spd_1;
             
  #ifdef VIEW_MOTORSPEED_DBG //''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''' x_dbg 
             if((cnt_view_dbg % 4) == 0){ 
               Serial_print1(" Increase: ");
               Serial_println(diff, DEC);
             }
  #endif  //'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''' x_dbg             
                                                 
          }else   
            sData.status_unit &= ~(1 << POS_BIT_MAX_CO2);  
        }      
       }else
         sData.status_unit &= ~(1 << POS_BIT_MAX_CO2);
         
       // Se siamo in zona di maggiore circolazione d'aria perchè un sensore ha si è avvicinato alla soglia, - Si nous sommes dans une zone de plus grande circulation d'air car un capteur s'est approché du seuil,
       // calcoliamo la maggiorazione del flusso. - on calcule l'augmentation du débit.
       if(sensorAir_op) { 
         unsigned short ref;
         // temp16 = (((unsigned short)spd_1 * 10) + 9 ) / temp_speed_motorsR; 
         ref = read_word_eeprom(ADDR_EEP(Set_StepMotorsCFS_CAF[idx]));
         ref = (unsigned short)((float)ref *0.255);

         temp16 = (((unsigned short)spd_1 * 10) + 9 ) / ref; 
         // per trovare la proporzione si fa:  new_setpoint = (setpoint_speed * increase_speed_RH_CO2) /10
         sData.increase_speed_RH_CO2 =  (byte)temp16;
         
         temp_speed_motorsR = spd_1;
         temp_speed_motorsF = spd_1;
       }else
           sData.increase_speed_RH_CO2 = 10; // Coefficiente moltiplicativo per 10,  10= 1.0 ed è l'incremento della velocità rispetto al set-poin a causa: RH alta, o CO2 alta, o VOCs alta.     
         
       // --- (4) RUN BOOST ---
       if (enab_func & (1 << ENAB_BOOST)) {
          count_Boost_on++;
          setPoint = (unsigned short)read_byte_eeprom(ADDR_EEP(Set_TimeBoost));  
          setPoint *= 50; // trasformo i minuti dell set point in secondi (la funzione gira OGNI 1,2 sec)
  
  #ifdef VIEW_MOTORSPEED_DBG //''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''' x_dbg 
          Serial_print1(" BOOST ON: ");
          Serial_println(count_Boost_on, DEC);
  #endif  //'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''' x_dbg                                                           
          
          if(count_Boost_on < setPoint) {
            temp_speed_motorsR = 255;
            temp_speed_motorsF = 255;
            sData.status_unit |= (1 << POS_BIT_BOOST_OPERATING); 
          }else {
              // superato il tempo di boost togliamo dalla config in eeprom la funzione boost.
              enab_func &=  ~(1 << ENAB_BOOST);
              count_Boost_on = 0;  // ATTENZIONE Corretto baco BOOST
              
              temp_08 = read_byte_eeprom(ADDR_EEP(cntUpdate_info));
              temp_08++;
  
              write_word_eeprom(ADDR_EEP(Enab_Fuction), enab_func);
              write_byte_eeprom(ADDR_EEP(cntUpdate_info), temp_08);
  
  #ifdef VIEW_MOTORSPEED_DBG //''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''' x_dbg 
              Serial_println1("Wr BOOST OFF ");
  #endif  //'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''' x_dbg              
                                                          
          }
       }else
          sData.status_unit &= ~(1 << POS_BIT_BOOST_OPERATING);   
      } // nuovo IF 1
    } // nuovo IF 2
       // --- (5) RUN IMBALANCE --- Aggiungiamo anche qui la necessità che DeFrost e CAP siano disiattivi
       if (((sData.status_unit & (1 << POS_BIT_DEFROST_OPERATING))== 0) && !(enab_func & (1<< ENAB_CAP)))
     {
       if (((sData.status_weekly & MSK_BIT_WEEKLY_RUN)  &&  (sData.status_weekly  & (0x03 << POS_BIT_SET_IMBAL))) || (enab_func & (1 << ENAB_IMBALANCE)))        
       {
            Serial_println1("passo dallo sblancio");
            // Aumento il ramo in funzione della percentuale impostata. 
            // Se raggiungimento del 100% del PWM ma non dello sbilanciamento, 
            // vado in sottrazione con l'altro ramo ;-)  
            float  sbilanc_fan = 0;
              byte sbil = (byte)read_byte_eeprom(ADDR_EEP(Set_Imbalance[0]));  
			int tempsb = 0;
			
			if (sbil & 0x80) {
				// Calcola il valore come complemento a due
				tempsb = sbil - 256;
				} else {
				// Il numero è positivo
				tempsb = sbil;
			}
                        
			unsigned short ref;
			// temp16 = (((unsigned short)spd_1 * 10) + 9 ) / temp_speed_motorsR;
			ref = read_word_eeprom(ADDR_EEP(Set_StepMotorsCFS_CAF[idx]));
			ref = (unsigned short)((float)ref *0.255);
			
						
						
            if(tempsb < 0) 
            { 
               // MAGGIORE ESTRAZIONE: Speed Motor R > Speed Motor F
               sbilanc_fan = (float)(100 + tempsb) / 100; 
               
               temp_speed_motorsF = (unsigned int)(sbilanc_fan * (float)ref);
               temp_speed_motorsR = ref;
                 
               sData.status_unit |= (1 << POS_BIT_IMBALANCE_OPERATING);   
                              
            }else if (tempsb  > 0) {
               // MAGGIORE IMMISSIONE: Speed Motor F > Speed Motor R
               sbilanc_fan = (float)(100 - tempsb) / 100;  
               
	            temp_speed_motorsF = ref;
	            temp_speed_motorsR = (unsigned int)(sbilanc_fan * (float)ref);
             
              
                sData.status_unit |= (1 << POS_BIT_IMBALANCE_OPERATING);        
            }else 
                sData.status_unit &= ~(1 << POS_BIT_IMBALANCE_OPERATING);  
         }
                 
  #ifdef VIEW_MOTORSPEED_DBG  //''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''' x_dbg          
            Serial_print1(" UNBAL ON: ");
            Serial_print(sbilanc_fan, 1);
            Serial_print1(" SetP: ");
            Serial_println(temp, DEC);
  #endif  //'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''' x_dbg                                                           
      
        }else  
           sData.status_unit &= ~(1 << POS_BIT_IMBALANCE_OPERATING);       
  //   } vecchio IF 1
 // }  vecchio IF 2
    
 
   // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -   
   // 7. CALCOLA il DEPOTENZIAMENTO in funzione della Categoria dei motori ed applicalo ai 2 rami.
   // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
   if(!(enab_func & (1<< ENAB_CAP)) && !(enab_func & (1<< ENAB_CAF))) {
       temp_speed_motorsR = (int)((float)temp_speed_motorsR * calcDepot());
       temp_speed_motorsF = (int)((float)temp_speed_motorsF * calcDepot());
   }
   
      
   // Nel caso che un accessorio di riscaldamento elettrico sia acceso, porta la velocità delle ventole 
   // ad un valore minimo tale per cui non si attiva l'allarme di mancaza di pressione o velocità dell'aria. 
   // sData.AccI2C[#].sts: .., bit[3]:1=ELECT/0=ACQUA, bit[2]:1=ACCESO/0=SPENTO, bit[1]:1=OPEARATIVO, bit[0]:1=Collegato
/*   if (((sData.AccI2C[ACC_I2C_PREHEATER].sts & 0x08) && (sData.AccI2C[ACC_I2C_PREHEATER].status_comand & CMD_ON)) || 
       ((sData.AccI2C[ACC_I2C_HEATER].sts & 0x08) && (sData.AccI2C[ACC_I2C_HEATER].status_comand & CMD_ON))) 
   {        
      minSpeedMotors = minSpeedMotors_HeaterON; // si parte con il 20% fino al 50%
      
      if(cnt_correct_minSpeedMotors_EHD_ON)
        cnt_correct_minSpeedMotors_EHD_ON--;
      else 
      {        
        // ogni 18 sec. se è presente ancora l'allarme incremento lo step min. fino al 75%   
        if(CkAlarm(ALM_EHD_PRESS) || CkAlarm(ALM_PEHD_PRESS) || cnt_no_Pressure_for_Electrical_Heater)  
        {
           minSpeedMotors_HeaterON += 13;    // increase +5%
           if(minSpeedMotors_HeaterON > 130) // max 50%
             minSpeedMotors_HeaterON = 130;
             
           cnt_correct_minSpeedMotors_EHD_ON = 14;  // 1.2 * 14 = 16.8 sec.
        }  
      }
         
   }else {
      minSpeedMotors_HeaterON = 100;   // 40% : accessorio elettrico
      cnt_correct_minSpeedMotors_EHD_ON = 0;
        
      if(count_switch_on < 25)  
        minSpeedMotors = MINSPEED_STARTUP;                        
      else   
        minSpeedMotors = MINSPEED;          
   }  */
   
   // Se c'è la presenza di fumo, bisogna dare la possibilità di spegnere un ramo in funzione del settaggio
   if((status_inputs != 0x83) && (status_inputs != 0x84)) { 
     if(temp_speed_motorsR < minSpeedMotors)
       temp_speed_motorsR = minSpeedMotors;
       
     if(temp_speed_motorsF < minSpeedMotors)
       temp_speed_motorsF = minSpeedMotors; 
   }  
   
   
#ifdef VIEW_MOTORSPEED_DBG  //''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''' x_dbg
   if((cnt_view_dbg % 4) == 0){
       Serial_println1(" ");
       Serial_print1("SpeedR:");
       Serial_print(temp_speed_motorsR, DEC);
       Serial_print1(" SpeedF:");
       Serial_println(temp_speed_motorsF, DEC);
       Serial_println1(" ");
   }
#endif   //'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''' x_dbg  
                                         

#endif // chiude   _MAPPING_FUNC
  
  //-------- ECO 30 : damperless bypass, if bypass is open the MOTOR_R MUST stay at minimum

  if (!(sData.status_unit &  (1 << POS_BIT_BYPASS_CLOSE)) && isDamperLessUnit()) {
	  temp_speed_motorsR = minSpeedMotors;
  }
   
   sData.speed_motors_R = temp_speed_motorsR;
   sData.speed_motors_F = temp_speed_motorsF;
    
   return 1;
  
}
