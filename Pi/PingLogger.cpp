//DB Specific declarations
sqlite3 *dbPingLogger;
char *zErrMsg = 0;
int rcPingLogger;
char *sqlPingLogger;
const char* data = "Callback function called";



  rcPingLogger = sqlite3_open("NetworkPingLogger", &db); // To be replaced by FarmHealth
  if ( rc ) {
    fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(dbPingLogger));
    return (0);
  }
  else {
    fprintf(stderr, "Opened database successfully\n");
  }




  printf_P(PSTR("************ PINGS ********** Message type %c\n\r"), header.type);

  unsigned long message;

// Retrieve the message from network
  network.read(header, &message, sizeof(unsigned long));


// For Testing purpose only
  asprintf(&sqlPingLogger, "Insert into Pings values (null, %i, datetime('now','localtime'), %s)",
  header.from_node, message);

  printf(sqlPingLogger);

  /* Execute SQL statement */
  rcPingLogger = sqlite3_exec(dbPingLogger, sqlPingLogger, callback, (void*)data, &zErrMsg);
  if ( rc != SQLITE_OK ) {
    fprintf(stderr, "SQL error: %s\n", zErrMsg); sqlite3_free(zErrMsg);
  }
  else {
    fprintf(stdout, "Operation done successfully\n");
  }


  sqlite3_close(dbPingLogger);
