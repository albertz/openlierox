#include "player.h"
#include "player_options.h"
#include "worm.h"
#ifndef DEDSERV
#include "viewport.h"
#endif
#include "ninjarope.h"

//#include <allegro.h>

using namespace std;

Player::Player(shared_ptr<PlayerOptions> options, BaseWorm* worm)
: BasePlayer(options, worm)
, aimingUp(false)
, aimingDown(false)
, changing(false)
, jumping(false)
, walkingRight(false)
, walkingLeft(false)
#ifndef DEDSERV
, m_viewport(0)
#endif
{

}

Player::~Player()
{
#ifndef DEDSERV
	delete m_viewport;
#endif
}

#ifndef DEDSERV
void Player::assignViewport(Viewport* viewport)
{
	m_viewport = viewport;
}
#endif

void Player::subThink()
{
	if ( m_worm )
	{
#ifndef DEDSERV
		if ( m_viewport ) m_viewport->interpolateTo(m_worm->getRenderPos(), m_options->viewportFollowFactor);
#endif
		
		if(changing && m_worm->getNinjaRopeObj()->active)
		{
			if(aimingUp)
			{
				m_worm->addRopeLength(-m_options->ropeAdjustSpeed);
			}
			if(aimingDown)
			{
				m_worm->addRopeLength(m_options->ropeAdjustSpeed);
			}
		}
		else
		{
			
			if (aimingUp && m_worm->aimSpeed > -m_options->aimMaxSpeed) 
			{
				m_worm->addAimSpeed(-m_options->aimAcceleration);
			}
			// No "else if" since we want to support precision aiming
			if (aimingDown && m_worm->aimSpeed < m_options->aimMaxSpeed)
			{
				m_worm->addAimSpeed(m_options->aimAcceleration);
			}
		}
		
		if(!aimingDown && !aimingUp)
		{
			// I placed this here since BaseWorm doesn't have access to aiming flags
			m_worm->aimSpeed *= m_options->aimFriction;
		}
	}
}

#ifndef DEDSERV
void Player::render()
{
	if ( m_viewport )
	{
		m_viewport->render(this);
	}
}
#endif

void Player::actionStart ( Actions action )
{
	switch (action)
	{
		case LEFT:
		{
			if ( m_worm )
			{
				if(changing)
				{
					m_worm->changeWeaponTo( m_worm->getWeaponIndexOffset(-1) );
				}
				else
				{
					BasePlayer::baseActionStart(BasePlayer::LEFT);
					walkingLeft = true;
					if ( walkingRight )
						BasePlayer::baseActionStart(BasePlayer::DIG);
				}
			}
		}
		break;
		
		case RIGHT:
		{
			if ( m_worm )
			{
				if(changing)
				{
					m_worm->changeWeaponTo( m_worm->getWeaponIndexOffset(1) );
				}
				else
				{
					BasePlayer::baseActionStart(BasePlayer::RIGHT);
					walkingRight = true;
					if ( walkingLeft )
						BasePlayer::baseActionStart(BasePlayer::DIG);
				}
			}
		}
		break;
		
		case FIRE:
		{
			if ( m_worm )
			{
				if(!changing)
					BasePlayer::baseActionStart(BasePlayer::FIRE);
			}
		}
		break;
		
		case JUMP:
		{
			if ( m_worm )
			{
				if ( m_worm->isActive() )
				{
					if (changing)
					{
						BasePlayer::baseActionStart(BasePlayer::NINJAROPE);
					}
					else
					{
						BasePlayer::baseActionStart(BasePlayer::JUMP);
						BasePlayer::baseActionStop(BasePlayer::NINJAROPE);
					}
					
					jumping = true;
				}else
				{
					BasePlayer::baseActionStart(BasePlayer::RESPAWN);
				}
			}
		}
		break;
		
		case UP:
		{
			if ( m_worm )
			{
				aimingUp = true;
			}
		}
		break;
		
		case DOWN:
		{
			if ( m_worm )
			{
				aimingDown = true;
			}
		}
		break;

		case CHANGE:
		{
			if ( m_worm )
			{
				m_worm->actionStart(Worm::CHANGEWEAPON);
				
				if (jumping)
				{
					BasePlayer::baseActionStart(BasePlayer::NINJAROPE);
					jumping = false;
				}
				else
				{
					m_worm->actionStop(Worm::FIRE); //TODO: Stop secondary fire also
					
					// Stop any movement
					m_worm->actionStop(Worm::MOVELEFT);
					m_worm->actionStop(Worm::MOVERIGHT);
					
				}
				
				changing = true;
			}
		}
		break;
		
		case ACTION_COUNT: break;
	}
}

void Player::actionStop ( Actions action )
{
	switch (action)
	{
		case LEFT:
		{
			if ( m_worm )
			{
				BasePlayer::baseActionStop(BasePlayer::LEFT);
				walkingLeft = false;
			}
		}
		break;
		
		case RIGHT:
		{
			if ( m_worm )
			{
				BasePlayer::baseActionStop(BasePlayer::RIGHT);
				walkingRight = false;
			}
		}
		break;
		
		case FIRE:
		{
			if ( m_worm )
			{
				BasePlayer::baseActionStop(BasePlayer::FIRE);
			}
		}
		break;
		
		case JUMP:
		{
			if ( m_worm )
			{
				BasePlayer::baseActionStop(BasePlayer::JUMP);
				jumping = false;
			}
		}
		break;
		
		case UP:
		{
			if ( m_worm )
			{
				aimingUp = false;
			}
		}
		break;
		
		case DOWN:
		{
			if ( m_worm )
			{
				aimingDown = false;
			}
		}
		break;
		
		case CHANGE:
		{
			if ( m_worm )
			{
				m_worm->actionStop(Worm::CHANGEWEAPON);

				changing = false;
			}
		}
		break;
		
		case ACTION_COUNT: break;
	}
}




