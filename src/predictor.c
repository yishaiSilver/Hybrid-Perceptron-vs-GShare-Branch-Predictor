//========================================================//
//  predictor.c                                           //
//  Source file for the Branch Predictor                  //
//                                                        //
//  Implement the various branch predictors below as      //
//  described in the README                               //
//========================================================//
#include <stdio.h>
#include <math.h>
#include "predictor.h"

//
// TODO:Student Information
//
const char *studentName = "Shmuel Silver";
const char *studentID   = "A15618184";
const char *email       = "ssilver@ucsd.edu";

//------------------------------------//
//      Predictor Configuration       //
//------------------------------------//

// Handy Global for use in output routines
const char *bpName[4] = { "Static", "Gshare",
                          "Tournament", "Custom" };

// int tournamentBits = 10;


int tournamentBits = 12;

int globalBits = 12;

//define number of bits required for indexing the BHT here. 
int phtIndexBits = 9;
int phtBits = 9; // Number of bits used for each entry of the PHT





int ghistoryBits = 12; // Number of bits used for Global History
int bpType;       // Branch Prediction Type
int verbose;


//------------------------------------//
//      Predictor Data Structures     //
//------------------------------------//

//
//TODO: Add your own Branch Predictor data structures here

uint8_t *tht;

//local history
uint8_t *pht;
uint8_t *bht_pht;

//gshare
uint8_t *bht_gshare;
uint64_t ghistory;





//------------------------------------//
//              gshare                //
//------------------------------------//
void init_gshare() {
 int bht_entries = 1 << ghistoryBits;
  bht_gshare = (uint8_t*)malloc(bht_entries * sizeof(uint8_t));
  printf("gshare bytes: %d\n", bht_entries);
  int i = 0;
  for(i = 0; i< bht_entries; i++){
    bht_gshare[i] = WN;
  }
  ghistory = 0;
}

uint8_t 
gshare_predict(uint32_t pc) {
  //get lower ghistoryBits of pc
  uint32_t bht_entries = 1 << ghistoryBits;
  uint32_t pc_lower_bits = pc & (bht_entries-1);
  uint32_t ghistory_lower_bits = ghistory & (bht_entries -1);
  uint32_t index = pc_lower_bits ^ ghistory_lower_bits;
  switch(bht_gshare[index]){
    case SN:
      return NOTTAKEN;
    case WN:
      return NOTTAKEN;
    case WT:
      return TAKEN;
    case ST:
      return TAKEN;
    default:
      printf("Warning: Undefined state of entry in GSHARE BHT!\n");
      return NOTTAKEN;
  }
}

void
train_gshare(uint32_t pc, uint8_t outcome) {
  //get lower ghistoryBits of pc
  uint32_t bht_entries = 1 << ghistoryBits;
  uint32_t pc_lower_bits = pc & (bht_entries-1);
  uint32_t ghistory_lower_bits = ghistory & (bht_entries -1);
  uint32_t index = pc_lower_bits ^ ghistory_lower_bits;

  //Update state of entry in bht based on outcome
  switch(bht_gshare[index]){
    case SN:
      bht_gshare[index] = (outcome==TAKEN)?WN:SN;
      break;
    case WN:
      bht_gshare[index] = (outcome==TAKEN)?WT:SN;
      break;
    case WT:
      bht_gshare[index] = (outcome==TAKEN)?ST:WN;
      break;
    case ST:
      bht_gshare[index] = (outcome==TAKEN)?ST:WT;
      break;
    default:
      printf("Warning: Undefined state of entry in GSHARE BHT!\n");
  }

  //Update history register
  ghistory = ((ghistory << 1) | outcome); 
}

void
cleanup_gshare() {
  free(bht_gshare);
}





void init_global() {
 int bht_entries = 1 << globalBits;
  bht_gshare = (uint8_t*)malloc(bht_entries * sizeof(uint8_t));
  printf("gshare bytes: %d\n", bht_entries);
  int i = 0;
  for(i = 0; i< bht_entries; i++){
    bht_gshare[i] = WN;
  }
  ghistory = 0;
}

uint8_t 
global_predict(uint32_t pc) {
  //get lower ghistoryBits of pc
  uint32_t bht_entries = 1 << globalBits;
  uint32_t pc_lower_bits = pc & (bht_entries-1);
  uint32_t ghistory_lower_bits = ghistory & (bht_entries -1);
  uint32_t index = ghistory_lower_bits;
  switch(bht_gshare[index]){
    case SN:
      return NOTTAKEN;
    case WN:
      return NOTTAKEN;
    case WT:
      return TAKEN;
    case ST:
      return TAKEN;
    default:
      printf("Warning: Undefined state of entry in GSHARE BHT!\n");
      return NOTTAKEN;
  }
}

void
train_global(uint32_t pc, uint8_t outcome) {
  //get lower ghistoryBits of pc
  uint32_t bht_entries = 1 << globalBits;
  uint32_t pc_lower_bits = pc & (bht_entries-1);
  uint32_t ghistory_lower_bits = ghistory & (bht_entries -1);
  uint32_t index = ghistory_lower_bits;

  //Update state of entry in bht based on outcome
  switch(bht_gshare[index]){
    case SN:
      bht_gshare[index] = (outcome==TAKEN)?WN:SN;
      break;
    case WN:
      bht_gshare[index] = (outcome==TAKEN)?WT:SN;
      break;
    case WT:
      bht_gshare[index] = (outcome==TAKEN)?ST:WN;
      break;
    case ST:
      bht_gshare[index] = (outcome==TAKEN)?ST:WT;
      break;
    default:
      printf("Warning: Undefined state of entry in GSHARE BHT!\n");
  }

  //Update history register
  ghistory = ((ghistory << 1) | outcome); 
}













//------------------------------------//
//                 pht                //
//------------------------------------//

void init_pht(){
  
  int pht_entries = 1 << phtIndexBits;
  pht = (uint8_t *)malloc(pht_entries * sizeof(uint8_t));

  int bht_entries = 1 << phtBits;
  bht_pht = (uint8_t *)malloc(bht_entries * sizeof(uint8_t));

  printf("pht bytes: %d\n", pht_entries + bht_entries);

  int i = 0;
  for(i = 0; i < bht_entries; i++){
    bht_pht[i] = N0;
  }
  
  for(i = 0; i < pht_entries; i++){
    pht[i] = 0;
  }
}

uint8_t 
pht_predict(uint32_t pc) {
  //get lower ghistoryBits of pc
  uint32_t pht_entries = 1 << phtIndexBits;
  uint32_t pc_lower_bits = pc & (pht_entries-1);
  uint32_t index = pht[pc_lower_bits];

  // printf("pc_lower_bits: %d\n", pc_lower_bits);
  // printf("index: %d\n", index);

  switch(bht_pht[index]){
    case N2:
    case N1:
    case N0:
      return NOTTAKEN;
    case T2:
    case T1:
    case T0:
      return TAKEN;
  }
}

void
train_pht(uint32_t pc, uint8_t outcome) {
  //get lower ghistoryBits of pc
  uint32_t pht_entries = 1 << phtIndexBits;
  uint32_t pc_lower_bits = pc & (pht_entries-1);
  uint32_t index = pht[pc_lower_bits];

  switch(bht_pht[index]){
    case N2: bht_pht[index] = (outcome==TAKEN)?N1:N2; break;
    case N1: bht_pht[index] = (outcome==TAKEN)?N0:N2; break;
    case N0: bht_pht[index] = (outcome==TAKEN)?T0:N1; break;
    case T0: bht_pht[index] = (outcome==TAKEN)?T1:N0; break;
    case T1: bht_pht[index] = (outcome==TAKEN)?T2:T0; break;
    case T2: bht_pht[index] = (outcome==TAKEN)?T2:T1; break;
  }

  //Update history register
  pht[pc_lower_bits] = ((pht[pc_lower_bits] << 1) | outcome ); 
}

void cleanup_pht(){
  free(pht);
  free(bht_pht);
}















  

//------------------------------------//
//             tournament             //
//------------------------------------//

void init_tourament(){
  init_global();
  init_pht();

  int tournament_entries = 1 << tournamentBits;
  tht = (uint8_t *)malloc(tournament_entries * sizeof(uint8_t));

  printf("tournament bytes: %d\n", tournament_entries);

  int i = 0;
  for(i = 0; i < tournament_entries; i++){
    tht[i] = WT;
  }

  // ghistory = 0;
}

uint8_t 
tournament_predict(uint32_t pc) {
  uint8_t globa_predict = global_predict(pc);
  uint8_t pht_prediction = pht_predict(pc);


  uint32_t tht_entries = 1 << tournamentBits;
  uint32_t ghistory_lower_bits = ghistory & (tht_entries -1);
  uint32_t choice = tht[ghistory_lower_bits];


  // return gshare_prediction;
  switch(choice){
    case SN:
    case WN:
      return globa_predict;
    case WT:
    case ST:
      return pht_prediction;
  }
}

void
train_tournament(uint32_t pc, uint8_t outcome) {
  uint8_t globa_predict = global_predict(pc);
  uint8_t pht_prediction = pht_predict(pc);

  uint32_t tht_entries = 1 << tournamentBits;
  uint32_t ghistory_lower_bits = ghistory & (tht_entries -1);
  uint32_t choice = tht[ghistory_lower_bits];

  train_global(pc, outcome);
  train_pht(pc, outcome);

  if (globa_predict == pht_prediction){
    return;
  }


  //Update state of entry in bht based on outcome
  switch(choice){
    case SN: // predicted gshare
      tht[ghistory_lower_bits] = (outcome==globa_predict)?SN:WN;
      break; 
    case WN: // predicted gshare
      tht[ghistory_lower_bits] = (outcome==globa_predict)?SN:WT;
      break;
    case WT: // predicted pht
      tht[ghistory_lower_bits] = (outcome==globa_predict)?WN:ST;
      break;
    case ST: // predicted pht
      tht[ghistory_lower_bits] = (outcome==globa_predict)?WT:ST;
  }
}

void cleanup_tournament(){
  cleanup_gshare();
  cleanup_pht();

  free(tht);
}

















uint8_t perceptronBits = 7;
uint8_t numPerceptronWeights = 23;
int8_t *perceptrons;
uint32_t ghr_perceptron;

//------------------------------------//
//                 pht                //
//------------------------------------//

void init_perceptron(){
  
  int perceptron_entries = 1 << perceptronBits;
  perceptrons = (int8_t *)malloc(perceptron_entries * numPerceptronWeights * sizeof(int8_t));
  
  // number of bytes used:
  printf("perceptron bytes: %d\n", perceptron_entries * numPerceptronWeights);
  // number of bits used:
  // printf("perceptron bits: %d\n", perceptron_entries * numPerceptronWeights * 8);

  ghr_perceptron = 0;
}

uint8_t 
perceptron_predict(uint32_t pc) {
  //get lower ghistoryBits of pc
  int perceptron_entries = 1 << perceptronBits;
  uint32_t pc_lower_bits = pc & (perceptron_entries-1);

  //get perceptron
  int8_t *perceptron = &perceptrons[pc_lower_bits * numPerceptronWeights];

  // take dot product of ghr and perceptron
  int8_t sum = 0;
  uint32_t ghr_lower_bits = ghr_perceptron;
  for(int i = 0; i < numPerceptronWeights; i ++){
    int8_t ghr_bit = ((ghr_lower_bits >> i) & 1) ? 1 : -1;
    if (i == numPerceptronWeights - 1){ ghr_bit = 1;}

    sum += ghr_bit * perceptron[i];
  }

  // print sum
  // printf("%d\n", sum);

  // return prediction
  return (sum >= 0) ? TAKEN : !TAKEN;
}

void
train_perceptron(uint32_t pc, uint8_t outcome) {
  //get lower ghistoryBits of pc
  int perceptron_entries = 1 << perceptronBits;
  uint32_t pc_lower_bits = pc & (perceptron_entries-1);

  //get perceptron
  int8_t *perceptron = &perceptrons[pc_lower_bits * numPerceptronWeights];

  // take dot product of ghr and perceptron
  int8_t sum = 0;
  uint32_t ghr_lower_bits = ghr_perceptron;
  for(int i = 0; i < numPerceptronWeights; i ++){
    int8_t ghr_bit = ((ghr_lower_bits >> i) & 1) ? 1 : -1;
    if (i == numPerceptronWeights - 1){ ghr_bit = 1;}

    sum += ghr_bit * perceptron[i];
  }

  int8_t sign = (sum >= 0) ? 1 : -1;
  int8_t t = (outcome == TAKEN) ? 1 : -1;
  
  sum = abs(sum);

  if (sign != t || sum <= (7)){
    for(int i = 0; i < numPerceptronWeights; i ++){
      int8_t ghr_bit = ((ghr_lower_bits >> i) & 1) ? 1 : -1;
      if (i == numPerceptronWeights - 1){ ghr_bit = 1;}

      // int8_t change = ghr_bit * t;
      // if (change < 0 && perceptron[i] > -7){
      //   perceptron[i] += change;
      // }
      // else if (change > 0 && perceptron[i] < 7){
      //   perceptron[i] += change;
      // }

      perceptron[i] = perceptron[i] + t * ghr_bit;
    }
  }

  ghr_perceptron = ((ghr_perceptron << 1) | outcome);
}





//------------------------------------//
//             tournament             //
//------------------------------------//

void init_custom(){
  init_gshare();
  init_perceptron();

  int tournament_entries = 1 << 9;
  tht = (uint8_t *)malloc(tournament_entries * sizeof(uint8_t));

  printf("tournament bytes: %d\n", tournament_entries);

  int i = 0;
  for(i = 0; i < tournament_entries; i++){
    tht[i] = WT;
  }
}

uint8_t 
custom_predict(uint32_t pc) {
  uint8_t gshare_prediction = gshare_predict(pc);
  uint8_t pht_prediction = perceptron_predict(pc);


  uint32_t tht_entries = 1 << 9;
  uint32_t pc_lower_bits = pc & (tht_entries-1);
  uint32_t choice = tht[pc_lower_bits];
 


  // return gshare_prediction;
  switch(choice){
    case SN:
    case WN:
      return gshare_prediction;
    case WT:
    case ST:
      return pht_prediction;
  }
}

void
train_custom(uint32_t pc, uint8_t outcome) {
  uint8_t gshare_prediction = gshare_predict(pc);
  uint8_t pht_prediction = perceptron_predict(pc);

  train_gshare(pc, outcome);
  train_perceptron(pc, outcome);

  if (gshare_prediction == pht_prediction){
    return;
  }

  uint32_t tht_entries = 1 << 9;
  uint32_t pc_lower_bits = pc & (tht_entries-1);
  uint32_t choice = tht[pc_lower_bits];

  //Update state of entry in bht based on outcome
  switch(choice){
    case SN: // predicted gshare
      tht[pc_lower_bits] = (outcome==gshare_prediction)?SN:WN;
      break; 
    case WN: // predicted gshare
      tht[pc_lower_bits] = (outcome==gshare_prediction)?SN:WT;
      break;
    case WT: // predicted pht
      tht[pc_lower_bits] = (outcome==gshare_prediction)?WN:ST;
      break;
    case ST: // predicted pht
      tht[pc_lower_bits] = (outcome==gshare_prediction)?WT:ST;
  }
}





void
init_predictor()
{
  printf("Initializing Predictor...\n");
  switch (bpType) {
    case GSHARE:
      init_gshare();
      break;
    case STATIC:
      init_perceptron();
      break;
    case TOURNAMENT:
      init_tourament();
      break;
    case CUSTOM:
      init_custom();
      break;
  }
  
}

// Make a prediction for conditional branch instruction at PC 'pc'
// Returning TAKEN indicates a prediction of taken; returning NOTTAKEN
// indicates a prediction of not taken
//
uint8_t
make_prediction(uint32_t pc)
{

  // Make a prediction based on the bpType
  switch (bpType) {
    case GSHARE:
      return gshare_predict(pc);
    case STATIC:
      return perceptron_predict(pc);
    case TOURNAMENT:
      return tournament_predict(pc);
    case CUSTOM:
      return custom_predict(pc);
    default:
      break;
  }

  // If there is not a compatable bpType then return NOTTAKEN
  return NOTTAKEN;
}

// Train the predictor the last executed branch at PC 'pc' and with
// outcome 'outcome' (true indicates that the branch was taken, false
// indicates that the branch was not taken)
//

void
train_predictor(uint32_t pc, uint8_t outcome)
{

  switch (bpType) {
    case GSHARE:
      return train_gshare(pc, outcome);
    case STATIC:
      return train_perceptron(pc, outcome);
    case TOURNAMENT:
      return train_tournament(pc, outcome);
    case CUSTOM:
      return train_custom(pc, outcome);
    default:
      break;
  }
}
