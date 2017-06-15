// 0 = Master
// 1-2 (02,05)   = Children of Master(00)
// 3,5 (012,022) = Children of (02)
// 4,6 (015,025) = Children of (05)
// 7   (032)     = Child of (02)
// 8,9 (035,045) = Children of (05)


#include <RF24/RF24.h>
#include <RF24Network/RF24Network.h>
#include <iostream>
#include <ctime>
#include <stdio.h>
#include <sqlite3.h>
#include <time.h>

// These are the Octal addresses that will be assigned
const uint16_t node_address_set[10] = { 00, 02, 05, 012, 015, 022, 025, 032, 035, 045 };

uint8_t NODE_ADDRESS = 0;  // Making this as MASTER

// CE Pin, CSN Pin, SPI Speed
RF24 radio(RPI_V2_GPIO_P1_15, RPI_V2_GPIO_P1_24, BCM2835_SPI_SPEED_8MHZ);

RF24Network network(radio);

// Data declarations
uint16_t this_node;                           // Our node address
const unsigned long interval = 1000; // ms       // Delay manager to send pings regularly.
unsigned long last_time_sent;

const short max_active_nodes = 10;            // Array of nodes we are aware of
uint16_t active_nodes[max_active_nodes];
short num_active_nodes = 0;
short next_ping_node_index = 0;

bool send_T(uint16_t to);                      // Prototypes for functions to send & handle messages
bool send_N(uint16_t to);

void handle_T(RF24NetworkHeader& header);
void handle_N(RF24NetworkHeader& header);
void handle_P(RF24NetworkHeader& header); // Handle Custom messages received
static int callback(void *data, int argc, char **argv, char **azColName)

void add_node(uint16_t node);

// Message structure
struct payload_t
{
  float temperature;
  float humidity;
  float	light;
  float fertility;
};

payload_t payload; // Structure for payload

// TEST - TBD
int counter = 11;

//DB Specific declarations
sqlite3 *db;
char *zErrMsg = 0;
int rc;
char *sql;
const char* data = "Callback function called";


/************************* START of main function **************************************/
int main(int argc, char** argv)
{

  // Opening the DB

  /* Open database */
  rc = sqlite3_open("FarmHealthTEST", &db); // To be replaced by FarmHealth
  if ( rc ) {
    fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
    return (0);
  }
  else {
    fprintf(stderr, "Opened database successfully\n");
  }

  //DB Operation - End

  this_node = node_address_set[NODE_ADDRESS];            // Which node are we?
  printf_P(PSTR(" THIS Node 0%o .\n\r"), this_node);

  // Bring up the RF network
  radio.begin();
  radio.setPALevel(RF24_PA_HIGH);
  network.begin( 100,  this_node ); /*(channel,node address)*/
  radio.printDetails(); // Print the RF network details

  // Run the ENDLESS loop
  while (1)
  {
    network.update();
    while ( network.available() )
    {
      RF24NetworkHeader header;                            // If so, take a look at it
      network.peek(header);

      switch (header.type) {                             // Dispatch the message to the correct handler.
        case 'T': handle_T(header); break;
        case 'N': handle_N(header); break;
        default:  handle_P(header); break;
      };
    }
    delay(2000);
  }

 // Pinging the network to check other nodes and receive other nodes status 
  unsigned long now = millis();                         // Send a ping to the next node every 'interval' ms
  if ( now - last_time_sent >= interval ) {
    last_time_sent = now;

    uint16_t to = 00;                                   // Who should we send to? By default, send to base

    if ( num_active_nodes ) {                           // Or if we have active nodes,
      to = active_nodes[next_ping_node_index++];      // Send to the next active node
      if ( next_ping_node_index > num_active_nodes ) { // Have we rolled over?
        next_ping_node_index = 0;                   // Next time start at the beginning
        to = 00;                                    // This time, send to node 00.
      }
    }

    bool ok;

    if ( this_node > 00 || to == 00 ) {                   // Normal nodes send a 'T' ping
      ok = send_T(to);
    } else {                                               // Base node sends the current active nodes out
      ok = send_N(to);
    }

    if (ok) {                                             // Notify us of the result
      printf_P(PSTR("%lu: APP Send ok\n\r"), millis());
    } else {
      printf_P(PSTR("%lu: APP Send failed\n\r"), millis());
      last_time_sent -= 100;                            // Try sending at a different time next time
    }
  }
  sqlite3_close(db);

  return 0;
}


/************************* END of main function **************************************/

/**
   Send a 'T' message, the current time
*/
bool send_T(uint16_t to)
{
  RF24NetworkHeader header(/*to node*/ to, /*type*/ 'T' /*Time*/);

  // The 'T' message that we send is just a ulong, containing the time
  unsigned long message = millis();
  printf_P(PSTR("---------------------------------\n\r"));
  printf_P(PSTR("%lu: APP Sending %lu to 0%o...\n\r"), millis(), message, to);
  return network.write(header, &message, sizeof(unsigned long));
}

/**
   Send an 'N' message, the active node list
*/
bool send_N(uint16_t to)
{
  RF24NetworkHeader header(/*to node*/ to, /*type*/ 'N' /*Time*/);

  printf_P(PSTR("---------------------------------\n\r"));
  printf_P(PSTR("%lu: APP Sending active nodes to 0%o...\n\r"), millis(), to);
  return network.write(header, active_nodes, sizeof(active_nodes));
}

/**
   Handle a 'T' message
   Add the node to the list of active nodes
*/
void handle_T(RF24NetworkHeader& header) {

  unsigned long message;                                                                      // The 'T' message is just a ulong, containing the time
  network.read(header, &message, sizeof(unsigned long));
  // The 'T' message is just a ulong, containing the time
  network.read(header, &payload, sizeof(payload));
  printf_P(PSTR("%lu: APP Received %lu from 0%o\n\r"), millis(), message, header.from_node);


  if ( header.from_node != this_node || header.from_node > 00 )                                // If this message is from ourselves or the base, don't bother adding it to the active nodes.
    add_node(header.from_node);
}

/**
   Handle an 'N' message, the active node list
*/
void handle_N(RF24NetworkHeader& header)
{
  static uint16_t incoming_nodes[max_active_nodes];

  network.read(header, &incoming_nodes, sizeof(incoming_nodes));
  printf_P(PSTR("%lu: APP Received nodes from 0%o\n\r"), millis(), header.from_node);

  int i = 0;
  while ( i < max_active_nodes && incoming_nodes[i] > 00 )
    add_node(incoming_nodes[i++]);
}

/**
   Add a particular node to the current list of active nodes
*/
void add_node(uint16_t node) {

  short i = num_active_nodes;                                    // Do we already know about this node?
  while (i--)  {
    if ( active_nodes[i] == node )
      break;
  }

  if ( i == -1 && num_active_nodes < max_active_nodes ) {        // If not, add it to the table
    active_nodes[num_active_nodes++] = node;
    printf_P(PSTR("%lu: APP Added 0%o to list of active nodes.\n\r"), millis(), node);
  }
}

void handle_P(RF24NetworkHeader& header)
{

  printf_P(PSTR("************ PAYLOAD ********** Message type %c\n\r"), header.type);

// Retrieve the payload  
  network.read(header, &payload, sizeof(payload));

// Print the retrieved payload values
  printf("Values received in the Payload: \n Temperature : %f \n Humidity : %f Light : %f and \n Fertility : %f ", 
   payload.temperature, payload.humidity, payload.light, payload.fertility);


  /* Create SQL statement */

  //sql = "Insert into Crop values (counter, 'somename', payload.humidity, datetime('now','localtime'), 25)";
  //asprintf(&sql, "Insert into MySoilHealth values (%i, %s', datetime('now','localtime'), %f, %f, %f, %f)", 
  //counter, header.from_node, payload.temperature, payload.humidity, payload.light, payload.fertility);
 
// For Testing purpose only 
  asprintf(&sql, "Insert into MySoilHealthTS values (null, %i, datetime('now','localtime'), %f, %f, %f, %f)", 
  header.from_node, payload.temperature, payload.humidity, payload.light, payload.fertility);
  
  //insert into MySoilHealthTS values (null , '99',datetime('now','localtime'),19.2,54.1,45.8,76.2);//(RecordID, SensorID, Timestamp, Temperature, Humidity, Light, Fertility)
  
  printf(sql);
  counter++;
  /* Execute SQL statement */
  rc = sqlite3_exec(db, sql, callback, (void*)data, &zErrMsg);
  if ( rc != SQLITE_OK ) {
    fprintf(stderr, "SQL error: %s\n", zErrMsg); sqlite3_free(zErrMsg);
  }
  else {
    fprintf(stdout, "Operation done successfully\n");
  }

}

// Printing the records from the DB(SQLite)
static int callback(void *data, int argc, char **argv, char **azColName) {
  int i; fprintf(stderr, "%s: ", (const char*)data); for (i = 0; i < argc; i++)
  {
    printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
  }
  printf("\n"); return 0;
}


