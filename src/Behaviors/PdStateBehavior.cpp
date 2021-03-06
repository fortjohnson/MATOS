//
// Created by David Johnson on 4/26/17.
//

#include <Belief.h>
#include "PdStateBehavior.h"

extern int g_agentID;


PdStateBehavior::PdStateBehavior(Goal g) : goal(g) {}


void PdStateBehavior::init(Beliefs beliefs, shared_ptr<AgentMonitor> oscMonitor) {

    m_beliefs = beliefs;

    // Initialize with OSC Input Function from Neighbors
    CallbackFunction stateIn = [&](const osc::ReceivedMessage& message) {
        osc::ReceivedMessageArgumentIterator arg = message.ArgumentsBegin();
        // Add (or update) a belief for Tempo message from a neighbor
        try{
            float state = arg->AsFloat();  // only one argument in a tempo message (if there are more they will get ignored)
            (*m_beliefs)[message.AddressPattern()] = make_shared<Belief>("state", state);
        }
        catch(const exception& e){
            cout << "Error Adding State Belief: " << e.what() << endl;
        }
    };
    oscMonitor->addFunction("/state/.*", stateIn);
}


void PdStateBehavior::processBeliefs(Beliefs beliefs, map<string, float> &blfParams) {

    vector<float> pdStates;

    for (auto beliefPair : *m_beliefs) {
        Belief b = *beliefPair.second;

        if (b.paramName == "state") {
            if (beliefPair.first == "/state/" + to_string(g_agentID)) {
                blfParams["myState"] = b.value;
            } else {
                pdStates.push_back(b.value);
            }
        }
        else if (b.paramName == "stateChgTime"){
            blfParams["stateChgTime"] = b.value;
        }
    }

    blfParams["currentTime"] = time(NULL) % 2592000;

    if (pdStates.size() > 0) {
        auto maxState = max_element(pdStates.begin(), pdStates.end());
        blfParams["maxWorldState"] = *maxState;
    } else {
        blfParams["maxWorldState"] = blfParams["myState"];
    }

    function<int(int, int)> randRange = [](int min, int max) -> int {
        srand(time(NULL));
        return rand() % (max - min) + min;
    };

    // TODO:  Make this part of Evaluation...
    blfParams["rand(25, 45)"] = randRange(25, 45);
    blfParams["rand(40, 65)"] = randRange(25, 45);

}


void PdStateBehavior::generateAction(map<string, float> &blfParams, vector<pair<bool, Goal> > &actionGoals) {

    try{
        bool goalMet = goal.evaluate(blfParams);

        if (goalMet){
        }
        else{
            pair<bool, Goal> p = make_pair(false, goal);
            actionGoals.push_back(p);
        }

    }
    catch(MissingParameterException e){
        cout << "Error: Unable to Evaluate Goal: "<< e.what() << endl;
    }
}
