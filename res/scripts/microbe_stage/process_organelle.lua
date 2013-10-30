--------------------------------------------------------------------------------
-- Class for Organelles capable of producing agents
--------------------------------------------------------------------------------
class 'ProcessOrganelle' (Organelle)

-- Constructor
function ProcessOrganelle:__init()
    self.buffers = {}
    self.inputAgents = {}
    self.outputAgents = {}
end

-- Add input agent to the recipy of the organelle
--
-- @param agentId
--  The agent to be used as input
--
-- @param amount
--  The amount of the agent needed
function ProcessOrganelle:addRecipyInput(agentId, amount)
    self.inputAgents[agentId] = amount
    self.buffers[agentId] = 0
end

-- Add output agent to the recipy of the organelle
--
-- @param agentId
--  The agent to be used as output
--
-- @param amount
--  The amount of the agent produced
function ProcessOrganelle:addRecipyOutput(agentId, amount)
    self.outputAgents[agent] = amount 
end

-- Store agent in buffer of processing organelle
--
-- @param agentId
--  The agent to be stored
--
-- @param amount
--  The amount to be stored
function ProcessOrganelle:storeAgent(agentId, amount)
    self.buffers[agentId] += amount
end

-- Checks if processing organelle has a given agent as an input
--
-- @param agentId
--  The agent to check for
function ProcessOrganelle:hasInputAgent(agentId)
    return inputAgents[agentid] ~= nil
end

-- Called by Microbe:update
--
-- Add output agent to the recipy of the organelle
--
-- @param microbe
--  The microbe containing the organelle
--
-- @param milliseconds
--  The time since the last call to update()
function ProcessOrganelle:update(microbe, milliseconds)
    Organelle:update(microbe, milliseconds)
    -- Attempt to produce
    for agentId,amount in pairs(self.inputAgents) do 
        if self.buffers[agentId] < self.inputAgents[agentId] then
            return -- not enough agent material for some agent type. Cannot produce.
        end
    end
    -- Sufficient agent material is available for production
    for agentId,amount in pairs(self.inputAgents) do 
        self.buffers[agentId] -= amount
    end
    for agentId,amount in pairs(self.outputAgents) do 
        microbe:storeAgent(agentId, amount)
    end
end

-- Buffer amounts aren't stored, could be added fairly easily
function ProcessOrganelle:storage()
    local storage = Organelle.storage(self)
    inputAgentsSt = StorageList()
    for agentId, amount in pairs(self.inputAgents) do
        inputStorage = StorageContainer()
        inputStorage:set("agentId", agentId)
        inputStorage:set("amount", amount)
        inputAgentsSt:append(inputStorage)
    end
    storage:set("inputAgents", inputAgentsSt)
    outputAgentsSt = StorageList()
    for agentId, amount in pairs(self.outputAgents) do
        outputStorage = StorageContainer()
        outputStorage:set("agentId", agentId)
        outputStorage:set("amount", amount)
        outputAgentsSt:append(outputStorage)
    end
    storage:set("outputAgents", outputAgentsSt)
    return storage
end

function Organelle:load(storage)
    Organelle.load(self, storage)
    local inputAgentsSt = storage:get("inputAgents", {})
    for i = 1,inputAgentsSt:size() do
        local inputStorage = inputAgentsSt:get(i)
        self:addRecipyInput(inputStorage:get("agentId", 0), inputStorage:get("amount", 0))
    end
    local outputAgentsSt = storage:get("outputAgents", {})
    for i = 1,outputAgentsSt:size() do
        local inputStorage = outputAgentsSt:get(i)
        self:addRecipyOutput(outputAgentsSt:get("agentId", 0), outputAgentsSt:get("amount", 0))
    end
end

