#include <iostream>
#include <unordered_map>
#include <fstream>
#include <vector>
#include <climits>
#include <string.h>
using namespace std;
#define COMMAND_SIZE 256
#define TOKEN_NUM 10

unordered_map<string, int> db;                          //store all the key-value pairs
unordered_map<int, int> counter;                        // count the # of keys with the same value

class TransactionBolck{
private:
    unordered_map<string, int> vars;                    //store the changes in each new transaction block
public:
    TransactionBolck(){
        
    }
    
    void set(string name, int value){
        
        if (db.find(name)==db.end()) {
            db[name]=value;
            counter[value]++;
            vars[name]=INT_MIN;                         //  INT_MIN represemts not exist in previous block
        }
        else {
            counter[db[name]]--;                       //update in the gloable maps
            if (counter[db[name]]==0) {
                counter.erase(db[name]);
            }
            if(vars.find(name)==vars.end())
                vars[name]=db[name];                  //store the change in its own map
            db[name]=value;
            counter[value]++;
        }
        
    }
    
    void unset(string name){
        if(db.find(name)==db.end()) return ;
        counter[db[name]]--;
        if (counter[db[name]]==0) {
            counter.erase(db[name]);
        }
        if(vars.find(name)==vars.end())
            vars[name]=db[name];
        db.erase(name);
    }
    unordered_map<string, int> getvars(){
        return vars;
    }
    
};

class DataBase{
private:
    vector<TransactionBolck *> blocks;                     //recording each new block
public:
    DataBase(){
    }
    void begin(){
        blocks.push_back(new TransactionBolck());          // adding a new block at the end
    }
    void set(string name, int value){
        if (blocks.size()==0) {
            if (db.find(name)==db.end()) {
                db[name]=value;
                counter[value]++;
            }
            else{
                counter[db[name]]--;
                if (counter[db[name]]==0) {
                    counter.erase(db[name]);
                }
                db[name]=value;
                counter[value]++;
            }
        }
        else
            blocks[blocks.size()-1]->set(name,value);
    }
    
    void unset(string name){
        if(blocks.size()==0){
            if (db.find(name)==db.end()) return;
            else {
                counter[db[name]]--;
                if (counter[db[name]]==0) {
                    counter.erase(db[name]);
                }
                db.erase(name);
            }
        }
        else{
            blocks[blocks.size()-1]->unset(name);
        }
    }
    
    bool commit(){
        if (blocks.size()<=0) {
            return false;
        }
        int size=blocks.size();
        while (size>0) {                               //discard all the blocks
            blocks.pop_back();
        }
        return true;
    }
    
    bool rollback(){
        if (blocks.size()<=0) {
            return false;
        }
        int pos=blocks.size()-1;                       // find the block we need to rollbak
        unordered_map<string, int>::iterator it;
        unordered_map<string, int> temp=blocks[pos]->getvars();
        for (it=temp.begin(); it!=temp.end(); it++) {     // using the iterator to traverse the map
            if (it->second==INT_MIN) {
                counter[db[it->first]]--;
                if (counter[db[it->first]]==0) {
                    counter.erase(db[it->first]);
                }
                db.erase(it->first);
            }
            else{
                if(db.find(it->first)!=db.end()){ //if the string is in the database, we need to update the counter
                    counter[db[it->first]]--;
                    if (counter[db[it->first]]==0) {
                        counter.erase(db[it->first]);
                    }
                }
                db[it->first]=it->second;
                counter[it->second]++;
            }
        }
        blocks.pop_back();   //discard the latest block
        return true;
    }
    
};


int main(int argc, const char * argv[]) {
    
    char input[COMMAND_SIZE];    // input command line
    string command[TOKEN_NUM];   // split the command line into tokens
    char* token;
    string name;
    int commandcount=0;
    DataBase *database= new DataBase();
    
    FILE * file=stdin;        // determine the input format
    if(argc>=2){
        file=fopen(argv[1], "r");
    }
    while (1) {
        memset(input,0,sizeof(input));
        fgets(input, COMMAND_SIZE, file);
        commandcount=0;
        token=strtok(input," \n");
        while (token!=NULL) {
            command[commandcount++]=token;
            token=strtok(NULL," \n");
        }
        if(command[0]=="GET"){
            name=command[1];
            if(db.find(name)==db.end()) printf("NULL\n");
            else{
                printf("%i\n",db[name]);
            }
        }
        else if(command[0]=="SET"){
            name=command[1];
            int value=stoi(command[2]);
            database->set(name, value);
        }
        else if(command[0]=="UNSET"){
            name=command[1];
            database->unset(name);
            
        }
        else if(command[0]=="NUMEQUALTO"){
            int value=stoi(command[1]);
            if (counter.find(value)==counter.end()) {
                printf("%i\n",0);
            }
            else printf("%i\n",counter[value]);
        }
        else if(command[0]=="END") break;
        else if(command[0]=="BEGIN"){
            database->begin();
        }
        else if(command[0]=="ROLLBACK"){
            if(!database->rollback()){
                printf("NO TRANSACTION\n");
            }
        }
        else if (command[0]=="COMMIT"){
            if(!database->commit()){
                printf("NO TRANSACTION\n");
            }
        }
        else {
            printf("Invalid command\n");
        }
        
    }
    
    
    return 0;
    
    
}