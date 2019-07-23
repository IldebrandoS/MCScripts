/*
 *  Copyright (C) 2017 - Elysium Project <http://elysium-project.org/>
 *
 *  Script rewritten by Zerix.
 *
 *  Notes: This is a proof of concept that writing scripts for SD2 can look, and feel better.
 *      Event based timers not only look cleaner, but they are easier to read and write for new/inexperienced
 *      developers.
 *  
 */

#include "scriptPCH.h"
#include "molten_core.h"

enum eSpells
{
    SpellImpendingDoom = 19702,             // Inflicts 2000 Shadow damage to nearby enemies after 10 sec. Radius: 40 yards.
    SpellCurse         = 19703,             // Curses nearby enemies, increasing the costs of their spells and abilities by 100% for 5 min. Radius: 40 yards.
    SpellShadowShock   = 19460              // Instantly lashes nearby enemies with dark magic, inflicting Shadow damage. Radius: 20 yards.
};

enum eEvents
{
    EventImpendingDoom = 1,
    EventCurse = 2,
    EventShadowShock = 3
};

struct boss_lucifronAI : public ScriptedAI
{
    explicit boss_lucifronAI(Creature* pCreature) : ScriptedAI(pCreature)
    {
        m_Instance = static_cast<ScriptedInstance*>(pCreature->GetInstanceData());
        boss_lucifronAI::Reset();
    }

	uint32 m_uiLuciCurseTimer;
    uint32 m_uiImpendingDoomTimer;

    void Reset() override
    {
        m_Events.Reset(); // wipe existing events or old timers are executed again on subsequent attempts
        m_uiLuciCurseTimer = urand(3,5);
        m_uiImpendingDoomTimer = urand(20,25);
		m_Events.ScheduleEvent(eEvents::EventImpendingDoom, Seconds(m_uiImpendingDoomTimer));       			  // 20 - 25 second initial cast
        m_Events.ScheduleEvent(eEvents::EventCurse, Seconds(m_uiImpendingDoomTimer + m_uiLuciCurseTimer));    // 3 seconds after impending doom
        m_Events.ScheduleEvent(eEvents::EventShadowShock, Seconds(6));         									  // Zerix: 6s Initial Cast, Repeats every 6s.

        if (m_Instance && m_creature->isAlive())
            m_Instance->SetData(TYPE_LUCIFRON, NOT_STARTED);
    }

    void Aggro(Unit* /*p_Who*/) override
    {
        if (m_Instance)
            m_Instance->SetData(TYPE_LUCIFRON, IN_PROGRESS);

        m_creature->SetInCombatWithZone();
    }

    void JustDied(Unit* /*p_Killer*/) override
    {
        if (m_Instance)
            m_Instance->SetData(TYPE_LUCIFRON, DONE);
    }

    void UpdateAI(const uint32 p_Diff) override
    {
        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        m_Events.Update(p_Diff);
        while (auto l_EventId = m_Events.ExecuteEvent())
        {
			m_uiLuciCurseTimer = urand(3,5);
			m_uiImpendingDoomTimer = urand(20,25);
            switch (l_EventId)
            {
                case eEvents::EventImpendingDoom:
                {
                    if (DoCastSpellIfCan(m_creature, eSpells::SpellImpendingDoom) == CAST_OK)
						
                        m_Events.Repeat(Seconds(m_uiImpendingDoomTimer));
                    else
                        m_Events.Repeat(Milliseconds(100));
                    break;
                }
                case eEvents::EventCurse:
                {
                    if (DoCastSpellIfCan(m_creature, eSpells::SpellCurse) == CAST_OK)
                        m_Events.Repeat(Seconds(m_uiImpendingDoomTimer + m_uiLuciCurseTimer));
                    else
                        m_Events.Repeat(Milliseconds(100));
                    break;
                }
                case eEvents::EventShadowShock:
                {          
						if (DoCastSpellIfCan(m_creature->getVictim(), eSpells::SpellShadowShock) == CAST_OK) //coppied this line from Gehannas tank shadow bolt                     
                            m_Events.Repeat(Seconds(6));
                    break;
                }
                default: break;
            }
        }

        DoMeleeAttackIfReady();
    }

private:
    EventMap m_Events;
    ScriptedInstance* m_Instance;
};

CreatureAI* GetAI_boss_lucifron(Creature* pCreature)
{
    return new boss_lucifronAI(pCreature);
}

void AddSC_boss_lucifron()
{
    Script *newscript;
    newscript = new Script;
    newscript->Name = "boss_lucifron";
    newscript->GetAI = &GetAI_boss_lucifron;
    newscript->RegisterSelf();
}

