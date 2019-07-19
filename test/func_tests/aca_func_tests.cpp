// c includes
#include <unistd.h>    /* for getopt */
#include "aca_comm_mgr.h"
#include "goalstate.pb.h"

using namespace std;
using aca_comm_manager::Aca_Comm_Manager;

// Defines
static char LOCALHOST[] = "localhost";
static char UDP[] = "udp";

// Global variables
bool g_execute_mode = false;
char *g_test_message = NULL;
char *g_rpc_server = NULL;
char *g_rpc_protocol = NULL;

using std::string;

static void aca_cleanup()
{
    // Optional:  Delete all global objects allocated by libprotobuf.
    google::protobuf::ShutdownProtobufLibrary();

    if (g_test_message != NULL){
        free(g_test_message);
        g_test_message = NULL;
    }

    if (g_rpc_server != NULL){
        free(g_rpc_server);
        g_rpc_server = NULL;
    }

    if (g_rpc_protocol != NULL){
        free(g_rpc_protocol);
        g_rpc_protocol = NULL;
    }    

    fprintf(stdout, "Program exiting, cleaning up...\n");
}

// function to handle ctrl-c and kill process
static void aca_signal_handler(int sig_num)
{
    fprintf(stdout, "Caught signal: %d\n", sig_num);

    // perform all the necessary cleanup here
    aca_cleanup();

    exit(sig_num);
}

int main(int argc, char *argv[])
{
    int option;
    int rc = EXIT_FAILURE;

    // Register the signal handlers
    signal(SIGINT, aca_signal_handler);
    signal(SIGTERM, aca_signal_handler);

    while ((option = getopt(argc, argv, "es:p:")) != -1)
    {
        switch (option)
        {
        case 'e':
            g_execute_mode = true;
            break;
        case 's':
            g_rpc_server = (char *)malloc(sizeof(char) * strlen(optarg));
            if (g_rpc_server != NULL)
            {
                strncpy(g_rpc_server, optarg, strlen(optarg));
            }
            else
            {
                fprintf(stdout, "Out of memory when allocating string with size: %lu.\n",
                              (sizeof(char) * strlen(optarg)));
                exit(EXIT_FAILURE);
            }
            break;
        case 'p':
            g_rpc_protocol = (char *)malloc(sizeof(char) * strlen(optarg));
            if (g_rpc_protocol != NULL)
            {
                strncpy(g_rpc_protocol, optarg, strlen(optarg));
            }
            else
            {
                fprintf(stdout, "Out of memory when allocating string with size: %lu.\n",
                              (sizeof(char) * strlen(optarg)));
                exit(EXIT_FAILURE);
            }
            break;
        default: /* the '?' case when the option is not recognized */
            fprintf(stderr, "Usage: %s\n"
                            "\t\t[-e excute command to transit daemon]\n"
                            "\t\t[-s transitd RPC server]\n"
                            "\t\t[-p transitd RPC protocol]\n",
                    argv[0]);
            exit(EXIT_FAILURE);
        }
    }

    // fill in the RPC server and protocol if it is not provided in command line arg
    if (g_rpc_server == NULL)
    {
        g_rpc_server = (char *)malloc(sizeof(char) * strlen(LOCALHOST));
        if (g_rpc_server != NULL)
        {
            strncpy(g_rpc_server, LOCALHOST, strlen(LOCALHOST));
        }
        else
        {
            fprintf(stdout, "Out of memory when allocating string with size: %lu.\n",
                            (sizeof(char) * strlen(LOCALHOST)));
            exit(EXIT_FAILURE);
        }
    }
    if (g_rpc_protocol == NULL)
    {
        g_rpc_protocol = (char *)malloc(sizeof(char) * strlen(UDP));
        if (g_rpc_protocol != NULL)
        {
            strncpy(g_rpc_protocol, UDP, strlen(UDP));
        }
        else
        {
            fprintf(stdout, "Out of memory when allocating string with size: %lu.\n",
                            (sizeof(char) * strlen(UDP)));
            exit(EXIT_FAILURE);
        }
    }

    // Verify that the version of the library that we linked against is
    // compatible with the version of the headers we compiled against.
    GOOGLE_PROTOBUF_VERIFY_VERSION;

    aliothcontroller::GoalState GoalState_builder;
    aliothcontroller::VpcState *new_vpc_states = GoalState_builder.add_vpc_states();
    new_vpc_states->set_operation_type(aliothcontroller::OperationType::CREATE);

    // this will allocate new VpcConfiguration, need to free it later
    aliothcontroller::VpcConfiguration *VpcConiguration_builder = new_vpc_states->mutable_configuration();
    VpcConiguration_builder->set_project_id("dbf72700-5106-4a7a-918f-a016853911f8");
    VpcConiguration_builder->set_id("99d9d709-8478-4b46-9f3f-2206b1023fd3");
    VpcConiguration_builder->set_name("SuperVpc");
    VpcConiguration_builder->set_cidr("192.168.0.0/24");

    string string_message;

    GoalState_builder.SerializeToString(&string_message);
    fprintf(stdout, "Serialized protobuf string: %s\n",
            string_message.c_str());


    uint byteSize = GoalState_builder.ByteSize(); 
    void *byteBuffer = malloc(byteSize);

    GoalState_builder.SerializeToArray(byteBuffer, byteSize);
    // want to find a better way to print it on screen
    // fprintf(stdout, "Serialized protobuf binary: %x\n", byteBuffer);



    Aca_Comm_Manager comm_manager;

    // rc = comm_manager.deserialize(**payload, parsed_struct);

/* if we want to execute the command
    if (rc == EXIT_SUCCESS)
    {
        rc = comm_manager.execute_command(parsed_struct);

        if (rc == EXIT_SUCCESS)
        {
            ACA_LOG_INFO("Successfully executed the network controller command");

            // TODO: need to free parsed_struct since we are done with it
        }
        else
        {
            ACA_LOG_ERROR("Unable to execute the network controller command: %d\n",
                            rc);
        }
    }
    */

/*
    if ((payload != nullptr) && (*payload != nullptr))
    {
        delete *payload;
    }
*/

    if (g_execute_mode)
    {
        // we can just execute command here
        // rc = comm_manager.process_messages();

    }


    if (byteBuffer != nullptr)
    {
        free(byteBuffer);
    }

    // free the allocated VpcConfiguration since we are done with it now
    new_vpc_states->clear_configuration();

   
    aca_cleanup();

    return rc;
}