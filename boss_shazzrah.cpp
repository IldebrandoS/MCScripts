/* Copyright (C) 2006 - 2010 ScriptDev2 <https://scriptdev2.svn.sourceforge.net/>
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/* ScriptData
SDName: Boss_Shazzrah
SD%Complete: 75
SDComment: Teleport NYI
SDCategory: Molten Core
EndScriptData */

#include "scriptPCH.h"
#include "molten_core.h"

enum
{
    SPELL_ARCANEEXPLOSION           = 19712,
    SPELL_SHAZZRAHCURSE             = 19713,
    SPELL_DEADENMAGIC               = 19714,
    SPELL_COUNTERSPELL              = 19715,
    SPELL_GATE_DUMMY                = 23138                 // effect spell: 23139
};

struct boss_shazzrahAI : public ScriptedAI
{
    boss_shazzrahAI(Creature* pCreature) : ScriptedAI(pCreature)
    {
        m_pInstance = (ScriptedInstance*)pCreature->GetInstanceData();
        Reset();
    }

    uint32 ArcaneExplosion_Timer;
    uint32 ShazzrahCurse_Timer;
    uint32 DeadenMagic_Timer;
    uint32 Countspell_Timer;
    uint32 Blink_Timer;

    ScriptedInstance* m_pInstance;

    void Reset()
    {
        ArcaneExplosion_Timer = 2000;
        ShazzrahCurse_Timer = 10000;
        DeadenMagic_Timer = 5000;
        Countspell_Timer = 15000;
        Blink_Timer = urand(30000, 35000);

        if (m_pInstance && m_creature->isAlive())
            m_pInstance->SetData(TYPE_SHAZZRAH, NOT_STARTED);
    }

    void Aggro(Unit* pWho)
    {
        if (m_pInstance)
            m_pInstance->SetData(TYPE_SHAZZRAH, IN_PROGRESS);
    }

    void JustDied(Unit* pKiller)
    {
        if (m_pInstance)
            m_pInstance->SetData(TYPE_SHAZZRAH, DONE);
    }

    void UpdateAI(const uint32 diff)
    {
        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        //ArcaneExplosion_Timer
        if (ArcaneExplosion_Timer < diff)
        {
            if (DoCastSpellIfCan(m_creature->getVictim(), SPELL_ARCANEEXPLOSION) == CAST_OK)
                ArcaneExplosion_Timer = 4000;
        }
        else ArcaneExplosion_Timer -= diff;

        //ShazzrahCurse_Timer
        if (ShazzrahCurse_Timer < diff)
        {
			if (DoCastSpellIfCan(m_creature->getVictim(), SPELL_SHAZZRAHCURSE, CF_AURA_NOT_PRESENT) == CAST_OK)
				ShazzrahCurse_Timer = urand(22000, 28000);
        }
        else ShazzrahCurse_Timer -= diff;

        //DeadenMagic_Timer
        if (DeadenMagic_Timer < diff)
        {
            if (DoCastSpellIfCan(m_creature, SPELL_DEADENMAGIC) == CAST_OK)
                DeadenMagic_Timer = urand(7000, 8000);
        }
        else DeadenMagic_Timer -= diff;

        //Countspell_Timer
        if (Countspell_Timer < diff)
        {
            if (DoCastSpellIfCan(m_creature->getVictim(), SPELL_COUNTERSPELL) == CAST_OK)
                Countspell_Timer = urand(15000, 20000);
        }
        else Countspell_Timer -= diff;

        //Blink_Timer
        if (Blink_Timer < diff)
        {
            // Teleporting him to a random gamer and casting Arcane Explosion after that.
            if (DoCastSpellIfCan(m_creature, SPELL_GATE_DUMMY, CF_TRIGGERED) == CAST_OK)
            {
                // manual, until added effect of dummy properly
                if (Unit* pTarget = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0, nullptr, SELECT_FLAG_PLAYER))
                {
                    DoResetThreat();
                    //if (Unit* pTarget = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 1))
                    m_creature->NearTeleportTo(pTarget->GetPositionX(), pTarget->GetPositionY(), pTarget->GetPositionZ(), m_creature->GetOrientation());
                    m_creature->Attack(pTarget, true);
                }

                Blink_Timer = urand(46000, 50000);
            }
        }
        else Blink_Timer -= diff;

        DoMeleeAttackIfReady();
    }
};
CreatureAI* GetAI_boss_shazzrah(Creature* pCreature)
{
    return new boss_shazzrahAI(pCreature);
}

void AddSC_boss_shazzrah()
{
    Script *newscript;
    newscript = new Script;
    newscript->Name = "boss_shazzrah";
    newscript->GetAI = &GetAI_boss_shazzrah;
    newscript->RegisterSelf();
}
