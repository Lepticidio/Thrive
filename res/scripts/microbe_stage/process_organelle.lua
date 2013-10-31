--------------------------------------------------------------------------------
-- Class for Organelles capable of producing agents
--------------------------------------------------------------------------------
class 'ProcessOrganelle' (Organelle)

-- Constructor
function ProcessOrganelle:__init(timeBetweenProcess)
    Organelle.__init(self)
    self.timeBetweenProcess = timeBetweenProcess
    self.processCooldown = 0
    self.buffers = {}
    self.inputAgents = {}
    self.outputAgents = {}
end


-- Overridded from Organelle:onAddedToMicrobe
function ProcessOrganelle:onAddedToMicrobe(microbe, q, r)
    Organelle.onAddedToMicrobe(self, microbe, q, r)
    microbe:addProcessOrganelle(self)
end


-- Set the minimum time that has to pass between agents are produced
-- 
-- @param milliseconds
--  The amount of time
function ProcessOrganelle:setTimeBetweenProcess(milliseconds)
    self.timeBetweenProcess = milliseconds
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
    self.outputAgents[agentId] = amount 
end


-- Store agent in buffer of processing organelle. 
-- This will force the organelle to store the agent, even if wantInputAgent is false.
-- It is recommended to check if wantInputAgent is true before calling.
--
-- @param agentId
--  The agent to be stored
--
-- @param amount
--  The amount to be stored
function ProcessOrganelle:storeAgent(agentId, amount)
    self.buffers[agentId] = self.buffers[agentId] + amount
end


-- Checks if processing organelle wants to store a given agent.
-- It wants an agent if it has that agent as input and its buffer relatively more full than it's process cooldown has left.
--
-- @param agentId
--  The agent to check for
-- 
-- @returns wantsAgent
--  true if the agent wants the agent, false if it can't use or doesn't want the agent
function ProcessOrganelle:wantsInputAgent(agentId)
    return (self.inputAgents[agentId] ~= nil and 
          self.processCooldown / (self.inputAgents[agentId] - self.buffers[agentId]) < (self.timeBetweenProcess / self.inputAgents[agentId])) -- calculate if it has enough buffered relative the amount of time left.
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
    Organelle.update(self, microbe, milliseconds)
    self.processCooldown = self.processCooldown - milliseconds
    if self.processCooldown < 0 then self.processCooldown = 0 end
    if self.processCooldown == 0 then
        -- Attempt to produce
        for agentId,amount in pairs(self.inputAgents) do 
            if self.buffers[agentId] < self.inputAgents[agentId] then
                return -- not enough agent material for some agent type. Cannot produce.
            end
        end
        -- Sufficient agent material is available for production
        self.processCooldown = self.timeBetweenProcess
        for agentId,amount in pairs(self.inputAgents) do 
            self.buffers[agentId] = self.buffers[agentId] - amount
        end
        for agentId,amount in pairs(self.outputAgents) do 
            microbe:storeAgent(agentId, amount)
        end
    end
end


-- Buffer amounts aren't stored, could be added fairly easily
function ProcessOrganelle:storage()
    local storage = Organelle.storage(self)
    storage:set("processCooldown", self.processCooldown)
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


function ProcessOrganelle:load(storage)
    Organelle.load(self, storage)
    self.processCooldown = storage:get("processCooldown", 0)
    local inputAgentsSt = storage:get("inputAgents", {})
    for i = 1,inputAgentsSt:size() do
        local inputStorage = inputAgentsSt:get(i)
        self:addRecipyInput(inputStorage:get("agentId", 0), inputStorage:get("amount", 0))
    end
    local outputAgentsSt = storage:get("outputAgents", {})
    for i = 1,outputAgentsSt:size() do
        local outputStorage = outputAgentsSt:get(i)
        self:addRecipyOutput(outputStorage:get("agentId", 0), outputStorage:get("amount", 0))
    end
end

