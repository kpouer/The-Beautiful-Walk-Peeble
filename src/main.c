#include <pebble.h>
#include "PebbleUI.h"
#include "ProgressBar.h"
#include "constants.h"



static Window *s_main_window;
static Layer *s_layer;

static TextLayer *title1;//THE
static TextLayer *title2;//BEAUTIFUL
static TextLayer *title3;//WALK
static TextLayer *total;
static TextLayer *nextCity;
static TextLayer *nextCityRemaining;
static TextLayer *nextCountry;
static TextLayer *nextCountryRemaining;
static TextLayer *nextGreatStep;
static TextLayer *nextGreatStepRemaining;
static TextLayer *messageLayer;
static BitmapLayer *topLayer;//top orange background
static int raceId;
static GColor mainColor;

static char walkid[250];

struct BeautifulWalkData
{
  char nextCity[48];
  struct ProgressBar cityProgress;
  char nextCountry[48];
  struct ProgressBar countryProgress;
  char nextGreatStep[48];
  struct ProgressBar greatStepProgress;
  struct ProgressBar totalProgress;
  int total;
};

static struct BeautifulWalkData data;

static void requestBeautifulWalk()
{
  APP_LOG(APP_LOG_LEVEL_INFO, "requestBeautifulWalk()");
  
  DictionaryIterator *iter;
  app_message_outbox_begin(&iter);
  app_message_set_context(&data);
  dict_write_int8(iter, KEY_QUERY, QUERY_TYPE_BEAUTIFULWALK);
  dict_write_int8(iter, KEY_RACEID, raceId);
  dict_write_cstring(iter, KEY_WALKID, walkid);
  
  // Send the message!
  app_message_outbox_send();
}  

static void update_display(Layer *layer, GContext *ctx)
{
  graphics_context_set_fill_color(ctx, mainColor);
  drawHorizontalBar(ctx, &data.cityProgress     , mainColor, GColorLightGray, BAR_SHIFT, CITY_BAR_Y     , BAR_MAX_LENGTH, BAR_HEIGHT, 5);
  drawHorizontalBar(ctx, &data.countryProgress  , mainColor, GColorLightGray, BAR_SHIFT, COUNTRY_BAR_Y  , BAR_MAX_LENGTH, BAR_HEIGHT, 5);
  drawHorizontalBar(ctx, &data.greatStepProgress, mainColor, GColorLightGray, BAR_SHIFT, NEXT_STEP_BAR_Y, BAR_MAX_LENGTH, BAR_HEIGHT, 5);
}

static void tap_handler(AccelAxisType axis, int32_t direction) 
{
  APP_LOG(APP_LOG_LEVEL_INFO, "tap_handler()");
  requestBeautifulWalk();
}

static void updateBackgroundColor()
{
  text_layer_set_background_color(title1      , mainColor);
  text_layer_set_background_color(title2      , mainColor);
  text_layer_set_background_color(title3      , mainColor);
  text_layer_set_background_color(total       , mainColor);
  text_layer_set_background_color(messageLayer, mainColor); 

  text_layer_set_text_color(nextCity              , mainColor);
  text_layer_set_text_color(nextCityRemaining     , mainColor);
  text_layer_set_text_color(nextCountry           , mainColor);
  text_layer_set_text_color(nextCountryRemaining  , mainColor);
  text_layer_set_text_color(nextGreatStep         , mainColor);
  text_layer_set_text_color(nextGreatStepRemaining, mainColor);
  
  bitmap_layer_set_background_color(topLayer, mainColor);
}

static void up_click_handler(ClickRecognizerRef recognizer, void *context) 
{
  APP_LOG(APP_LOG_LEVEL_INFO, "up_click_handler()");
  if (raceId != 0)
  {
    layer_add_child(s_layer, text_layer_get_layer(nextGreatStep));
    layer_add_child(s_layer, text_layer_get_layer(nextGreatStepRemaining));
    mainColor = COLOR_FALLBACK(GColorOrange, GColorBlack);
    updateBackgroundColor();
    raceId = 0;
  }
  requestBeautifulWalk();
}

static void down_click_handler(ClickRecognizerRef recognizer, void *context) 
{
  APP_LOG(APP_LOG_LEVEL_INFO, "down_click_handler()");
  if (raceId != 1)
  {
    layer_remove_from_parent(text_layer_get_layer(nextGreatStep));
    layer_remove_from_parent(text_layer_get_layer(nextGreatStepRemaining));
    mainColor = COLOR_FALLBACK(GColorRed, GColorBlack);
    updateBackgroundColor();
    raceId = 1;
  }

  requestBeautifulWalk();
}

static void click_config_provider(void *context) 
{
  window_single_click_subscribe(BUTTON_ID_UP, (ClickHandler)up_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, (ClickHandler)down_click_handler);
}

static int remainingDistance(struct ProgressBar *progressBar)
{
  return (progressBar->total - progressBar->value) / 100;
}

static void updateScreen()
{
  static char totalBuffer[10];
  static char nextCityDistance[8];
  static char nextCountryDistance[8];
  static char nextStepDistance[8];

  snprintf(totalBuffer, sizeof(totalBuffer), "%d km", (data.total/100));
  text_layer_set_text(nextCity     , data.nextCity);
  text_layer_set_text(nextCountry  , data.nextCountry);
  text_layer_set_text(nextGreatStep, data.nextGreatStep);
  text_layer_set_text(total        , totalBuffer);
  
  snprintf(nextCityDistance   , sizeof(nextCityDistance)   , "%dkm", remainingDistance(&data.cityProgress));
  snprintf(nextCountryDistance, sizeof(nextCountryDistance), "%dkm", remainingDistance(&data.countryProgress));
  snprintf(nextStepDistance   , sizeof(nextStepDistance)   , "%dkm", remainingDistance(&data.greatStepProgress));

  text_layer_set_text(nextCityRemaining     , nextCityDistance);
  text_layer_set_text(nextCountryRemaining  , nextCountryDistance);
  text_layer_set_text(nextGreatStepRemaining, nextStepDistance);
}

static void receivedBeautifulWalk(DictionaryIterator *iterator)
{
  APP_LOG(APP_LOG_LEVEL_INFO, "receivedBeautifulWalk()");
  Tuple *nextCityTuple         = dict_find(iterator, KEY_NEXTCITY);
  Tuple *nextCityDoneTuple     = dict_find(iterator, KEY_NEXTCITYDONE);
  Tuple *nextCityTotalTuple    = dict_find(iterator, KEY_NEXTCITYTOTAL);
  Tuple *nextCountryTuple      = dict_find(iterator, KEY_NEXTCOUNTRY);
  Tuple *nextCountryDoneTuple  = dict_find(iterator, KEY_NEXTCOUNTRYDONE);
  Tuple *nextCountryTotalTuple = dict_find(iterator, KEY_NEXTCOUNTRYTOTAL);
  Tuple *nextStepTuple         = dict_find(iterator, KEY_NEXTSTEP);
  Tuple *nextStepDoneTuple     = dict_find(iterator, KEY_NEXTSTEPDONE);
  Tuple *nextStepTotalTuple    = dict_find(iterator, KEY_NEXTSTEPTOTAL);
  Tuple *totalDone             = dict_find(iterator, KEY_TOTALDONE);

  bool error = false;
  if (nextCityTuple && nextCityDoneTuple && nextCityTotalTuple && 
      nextCountryTuple && nextCountryDoneTuple && nextCountryTotalTuple && totalDone)
  {
    data.cityProgress.value = nextCityDoneTuple->value->uint32;
    data.cityProgress.total = nextCityTotalTuple->value->uint32;
    if (strlen(nextCityTuple->value->cstring) > sizeof(data.nextCity))
      strncpy(data.nextCity, nextCityTuple->value->cstring, sizeof(data.nextCity));
    else
      strcpy(data.nextCity, nextCityTuple->value->cstring);
    APP_LOG(APP_LOG_LEVEL_INFO, "next city      :%s", data.nextCity);
    
    data.countryProgress.value = nextCountryDoneTuple->value->uint32;
    data.countryProgress.total = nextCountryTotalTuple->value->uint32;
    if (strlen(nextCityTuple->value->cstring) > sizeof(data.nextCity))
      strncpy(data.nextCountry, nextCountryTuple->value->cstring, sizeof(data.nextCountry));
    else
      strcpy(data.nextCountry, nextCountryTuple->value->cstring);
    APP_LOG(APP_LOG_LEVEL_INFO, "next country   :%s", data.nextCountry);

    data.greatStepProgress.value = nextStepDoneTuple->value->uint32;
    data.greatStepProgress.total = nextStepTotalTuple->value->uint32;
   
    data.total = totalDone->value->int32;
        
    persist_write_data(KEY_PERSISTDATA, &data, sizeof(data));
    text_layer_set_text(messageLayer, "");
  }
  else
  {
     APP_LOG(APP_LOG_LEVEL_ERROR, "receivedBeautifulWalk() wrong data1");
    error = true;
  }
  
  if (nextStepTuple && nextStepDoneTuple && nextStepTotalTuple)
  {
    data.greatStepProgress.value = nextStepDoneTuple->value->uint32;
    data.greatStepProgress.total = nextStepTotalTuple->value->uint32;
    if (strlen(nextCityTuple->value->cstring) > sizeof(data.nextCity))
      strncpy(data.nextGreatStep, nextStepTuple->value->cstring, sizeof(data.nextGreatStep));
    else
      strcpy(data.nextGreatStep, nextStepTuple->value->cstring);
    APP_LOG(APP_LOG_LEVEL_INFO, "next Great Step:%s", data.nextGreatStep);
  }
  else
  {
    if( raceId == 0)
    {
      APP_LOG(APP_LOG_LEVEL_ERROR, "receivedBeautifulWalk() wrong data2");
      error = true;
    }
  }
  
  if (error)
  {
    APP_LOG(APP_LOG_LEVEL_ERROR, "receivedBeautifulWalk() wrong data");
    text_layer_set_text(messageLayer, "Erreur de chargement des données");
  }
  else
  {
    updateScreen();
    layer_mark_dirty(s_layer);
  }
}
  
static void inbox_received_callback(DictionaryIterator *iterator, void *context)
{
  APP_LOG(APP_LOG_LEVEL_INFO, "inbox_received_callback()");
  
  Tuple *walkidTuple = dict_find(iterator, KEY_WALKID);
  if (walkidTuple)
  {
    strcpy(walkid, walkidTuple->value->cstring);
    persist_write_string(KEY_PERSISTDATA_WALKID, walkid);
    requestBeautifulWalk();
    return;
  }
  if (iterator == NULL)
  {
    APP_LOG(APP_LOG_LEVEL_ERROR, "inbox_received_callback() iterator is null");
    requestBeautifulWalk();
    return;
  }
  
  if (!context)
  {
    // it was the init message
    requestBeautifulWalk();
    return;
  }
  
  if (context == &data)
  {
    receivedBeautifulWalk(iterator);
  }
  else
  {
    APP_LOG(APP_LOG_LEVEL_INFO, "no key query");  
  }
}

static void inbox_dropped_callback(AppMessageResult reason, void *context)
{
  APP_LOG(APP_LOG_LEVEL_ERROR, "Message dropped!");
  text_layer_set_text(messageLayer, "Dropped!");
}

static void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context)
{
  APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox sent failed!");
  text_layer_set_text(messageLayer, "Failed!");
}

static void outbox_sent_callback(DictionaryIterator *iterator, void *context)
{
  APP_LOG(APP_LOG_LEVEL_INFO, "Outbox send success!");
  text_layer_set_text(messageLayer, "Chargement...");
}

static void main_window_load(Window *window)
{
  APP_LOG(APP_LOG_LEVEL_INFO, "main_window_load()");
  mainColor = COLOR_FALLBACK(GColorOrange, GColorBlack);

  Layer *window_layer = window_get_root_layer(window);
  s_layer = layer_create(layer_get_bounds(window_layer));
  GRect bounds = layer_get_bounds(s_layer);
  
  topLayer = bitmap_layer_create(GRect(0,0, bounds.size.w, 50+25));
  bitmap_layer_set_background_color(topLayer, mainColor);
    
  title1                 = text_layer_create(GRect(2                          , 0          , bounds.size.w-            2, FONT_TITLE_HEIGHT));
  title2                 = text_layer_create(GRect(2                          , TITLE_2_Y  , bounds.size.w-2            , FONT_TITLE_HEIGHT));
  title3                 = text_layer_create(GRect(2                          , TITLE_3_Y  , bounds.size.w-2            , FONT_TITLE_HEIGHT));
  nextCity               = text_layer_create(GRect(0                          , CITY_Y     , bounds.size.w-CITY_SPLIT   , FONT_DEFAULT_HEIGHT));
  nextCityRemaining      = text_layer_create(GRect(bounds.size.w-CITY_SPLIT   , CITY_Y     , CITY_SPLIT                 , FONT_DEFAULT_HEIGHT));
  nextCountry            = text_layer_create(GRect(0                          , COUNTRY_Y  , bounds.size.w-COUNTRY_SPLIT, FONT_DEFAULT_HEIGHT));
  nextCountryRemaining   = text_layer_create(GRect(bounds.size.w-COUNTRY_SPLIT, COUNTRY_Y  , COUNTRY_SPLIT              , FONT_DEFAULT_HEIGHT));
  nextGreatStep          = text_layer_create(GRect(0                          , NEXT_STEP_Y, bounds.size.w-STEP_SPLIT   , FONT_DEFAULT_HEIGHT));
  nextGreatStepRemaining = text_layer_create(GRect(bounds.size.w-STEP_SPLIT   , NEXT_STEP_Y, STEP_SPLIT                 , FONT_DEFAULT_HEIGHT));
  total                  = text_layer_create(GRect(64                         , 0          , bounds.size.w - 64         , 18));
  messageLayer           = text_layer_create(GRect(50                         , CITY_Y - 22, bounds.size.w - 50         , 22));
  initTextLayer(title1                , "THE"       , GColorWhite, mainColor  , GTextAlignmentLeft , fonts_get_system_font(FONT_TITLE));
  initTextLayer(title2                , "BEAUTIFUL" , GColorWhite, mainColor  , GTextAlignmentLeft , fonts_get_system_font(FONT_TITLE));
  initTextLayer(title3                , "WALK"      , GColorWhite, mainColor  , GTextAlignmentLeft , fonts_get_system_font(FONT_TITLE));
  initTextLayer(nextCity              , ""          , mainColor  , GColorWhite, GTextAlignmentLeft , fonts_get_system_font(FONT_DEFAULT));
  initTextLayer(nextCityRemaining     , ""          , mainColor  , GColorWhite, GTextAlignmentRight, fonts_get_system_font(FONT_DEFAULT));
  initTextLayer(nextCountry           , ""          , mainColor  , GColorWhite, GTextAlignmentLeft , fonts_get_system_font(FONT_DEFAULT));
  initTextLayer(nextCountryRemaining  , ""          , mainColor  , GColorWhite, GTextAlignmentRight, fonts_get_system_font(FONT_DEFAULT));
  initTextLayer(nextGreatStep         , ""          , mainColor  , GColorWhite, GTextAlignmentLeft , fonts_get_system_font(FONT_DEFAULT));
  initTextLayer(nextGreatStepRemaining, ""          , mainColor  , GColorWhite, GTextAlignmentRight, fonts_get_system_font(FONT_DEFAULT));
  initTextLayer(total                 , ""          , GColorWhite, mainColor  , GTextAlignmentRight, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
  initTextLayer(messageLayer          , ""          , GColorWhite, mainColor  , GTextAlignmentRight, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
  layer_add_child(window_layer, s_layer);
  layer_add_child(s_layer, bitmap_layer_get_layer(topLayer));
  layer_add_child(s_layer, text_layer_get_layer(title1));
  layer_add_child(s_layer, text_layer_get_layer(title2));
  layer_add_child(s_layer, text_layer_get_layer(title3));
  layer_add_child(s_layer, text_layer_get_layer(nextCity));
  layer_add_child(s_layer, text_layer_get_layer(nextCityRemaining));
  layer_add_child(s_layer, text_layer_get_layer(nextCountry));
  layer_add_child(s_layer, text_layer_get_layer(nextCountryRemaining));
  layer_add_child(s_layer, text_layer_get_layer(nextGreatStep));
  layer_add_child(s_layer, text_layer_get_layer(nextGreatStepRemaining));
  layer_add_child(s_layer, text_layer_get_layer(total));
  layer_add_child(s_layer, text_layer_get_layer(messageLayer));
  layer_set_update_proc(s_layer, update_display);
  window_set_click_config_provider(window, (ClickConfigProvider) click_config_provider);
}

static void main_window_unload(Window *window)
{
  APP_LOG(APP_LOG_LEVEL_INFO, "main_window_unload()");
  layer_destroy(s_layer);
  bitmap_layer_destroy(topLayer);
  text_layer_destroy(title1);
  text_layer_destroy(title2);
  text_layer_destroy(title3);
  text_layer_destroy(nextCity);
  text_layer_destroy(nextCityRemaining);
  text_layer_destroy(nextCountry);
  text_layer_destroy(nextCountryRemaining);
  text_layer_destroy(nextGreatStep);
  text_layer_destroy(nextGreatStepRemaining);
  text_layer_destroy(messageLayer);
}

static void init() 
{
  APP_LOG(APP_LOG_LEVEL_INFO, "init()");
  raceId = 0;
  
  int storedStructVersion = persist_read_int(KEY_PERSISTDATA_VERSION);
  int dataLoaded;
  int hasWid = persist_read_string(KEY_PERSISTDATA_WALKID, walkid, sizeof(walkid));
  if (hasWid == E_DOES_NOT_EXIST)
  {
    strcpy(walkid, DEFAULT_WALK_ID);
  }
  if (storedStructVersion != STRUCT_VERSION)
    dataLoaded = E_DOES_NOT_EXIST;// version of structure is incompatible
  else
    dataLoaded = persist_read_data(KEY_PERSISTDATA, &data, sizeof(data));
  
  if (E_DOES_NOT_EXIST == dataLoaded)
  {
    APP_LOG(APP_LOG_LEVEL_INFO, "Unable to reload persistent data");
    memset(data.nextCity     , 0, sizeof(data.nextCity));
    memset(data.nextCountry  , 0, sizeof(data.nextCountry));
    memset(data.nextGreatStep, 0, sizeof(data.nextGreatStep));
    data.total = 0;
    resetProgressBar(&data.cityProgress);
    resetProgressBar(&data.countryProgress);
    resetProgressBar(&data.greatStepProgress);
    resetProgressBar(&data.totalProgress);
    persist_write_int(KEY_PERSISTDATA_VERSION, STRUCT_VERSION);
    persist_write_data(KEY_PERSISTDATA, &data, sizeof(data));
  } 
  
  app_message_register_inbox_received(inbox_received_callback);
  app_message_register_inbox_dropped(inbox_dropped_callback);
  app_message_register_outbox_failed(outbox_failed_callback);
  app_message_register_outbox_sent(outbox_sent_callback);
  
  app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());
  
  s_main_window = window_create();
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload,
  });
  window_set_background_color(s_main_window, GColorWhite);
  window_stack_push(s_main_window, true);
  accel_tap_service_subscribe(tap_handler);  
  if (E_DOES_NOT_EXIST != dataLoaded)
  {
    APP_LOG(APP_LOG_LEVEL_INFO, "Persistent data loaded");
    updateScreen();
  }
}

static void deinit()
{
  APP_LOG(APP_LOG_LEVEL_INFO, "deinit()");
  window_destroy(s_main_window);
  accel_tap_service_unsubscribe();
}

int main(void)
{
  init();
  app_event_loop();
  deinit();
}