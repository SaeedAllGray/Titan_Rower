#include <Arduino.h>
#include <AWS.h>

void taskOne( void * parameter);
void taskTwo( void * parameter);
#define LED_BOARD 2 //change here the pin of the board to V2

myawsclass aws = myawsclass();

void setup(){
  pinMode(LED_BOARD, OUTPUT);
  Serial.begin(921600);
  delay(1000);

  aws.connectAWS();
  xTaskCreate(
                    taskOne,          /* Task function. */
                    "TaskOne",        /* String with name of task. */
                    /* gonna crash if the stack size is 512 only */
                    1024,             /* Stack size in bytes. */
                    NULL,             /* Parameter passed as input of the task */
                    1,                /* Priority of the task. */
                    NULL);            /* Task handle. */

  xTaskCreate(
                    taskTwo,          /* Task function. */
                    "TaskTwo",        /* String with name of task. */
                    1024,             /* Stack size in bytes. */
                    NULL,             /* Parameter passed as input of the task */
                    1,                /* Priority of the task. */
                    NULL);            /* Task handle. */
}

void loop()
{
  delay(1000);
}

void taskOne( void * parameter )
{
    //example of a task that executes for some time and then is deleted
    for( int i = 0; i < 10; i++ )
    {
      Serial.println("Hello from task 1");
      
      //Switch on the LED
      digitalWrite(LED_BOARD, HIGH); 
      // Pause the task for 1000ms
      //delay(1000); //This delay doesn't give a chance to the other tasks to execute
      vTaskDelay(1000 / portTICK_PERIOD_MS); //this pauses the task, so others can execute
      // Switch off the LED
      digitalWrite(LED_BOARD, LOW);
      // Pause the task again for 500ms
      vTaskDelay(500 / portTICK_PERIOD_MS);
    }
    Serial.println("Ending task: 1");
    vTaskDelete( NULL );
}
 
void taskTwo( void * parameter)
{
    //create an endless loop so the task executes forever
    while (true)
    {
      Serial.println("Hello from task: 2");
      aws.publishMessage(8964);
      aws.stayConnected();
      vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
    Serial.println("Ending task 2"); //should not reach this point but in case...
    vTaskDelete( NULL );
}
