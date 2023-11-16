using Robocode;
using System;
using Robocode.Util;
using System.Drawing;
using System.Collections.Generic;
using System.Threading;
using System.Linq;

namespace CAP4053.Student
{
    public class CoolGuy : TeamRobot
    {

        const int invalid = -1;
        const int sweep = 0;
        const int find = 1;
        const int fire = 2;
        const int track = 3;
        //aliases for states

        bool shouldFire;
        double opponentEnergy = -1;

        string targetName = "";

        const double highPower = 3;
        const double lowPower = 2;
        double power = highPower;

        int state = sweep;

        long latestScanTurn = 0;

        long foundOneOpponent = 0;

        bool foundTarget = false;

        Dictionary<string, ScannedRobotEvent> opponents;
        StateMachine finiteStateMachine;
        List<(double, long)> headingList;

        public class StateMachine
        {
            public class State
            {
                public delegate void InitFunction();
                public delegate void MovementFunction();
                public delegate void TargetFunction(ScannedRobotEvent e);
                public delegate (bool, int) TransitionFunction();

                public InitFunction init;
                public MovementFunction move;
                public TargetFunction target;
                public TransitionFunction trans;
                //States have four functions associated with them. An init function runs once on startup
                //Movement function is called every frame, determines how the robot will move
                //Target function is called whenever the robot scans an enemy
                //Transition function also runs every frame, determines if it should transition and what to
                public State(InitFunction i, MovementFunction m, TargetFunction t, TransitionFunction ts)
                {
                    init = i;
                    move = m;
                    target = t;
                    trans = ts;
                }
            }

            List<State> states;
            State currentState;

            public StateMachine(State s)
            {
                states = new List<State>();
                states.Add(s);
                currentState = s;
            }
            public void movementBehavior()
            {
                currentState.move();
            }

            public void scanBehavior(ScannedRobotEvent e)
            {
                currentState.target(e);
            }

            public void addState(State s)
            {
                states.Add(s);
            }
            public void transition()
            {
                (bool, int) transitionInfo = currentState.trans();
                if (transitionInfo.Item1 == true)
                {
                    currentState = states[transitionInfo.Item2];
                    currentState.init();
                }
            }
            //wrappers to call our current's states relevant functions. 
            //no wrapper for init, since it only happens once at the beginning it's called in transition if it actually transitions
        }

        Random rand = new Random();
        void discoveryStatesInit()
        {
            foundTarget = false;
            SetTurnRadarRight(360);
        }
        //set the radar to scan the full arena, needed for both sweep and locate

        void fireInit()
        {
            return;
        }
        //doesn't need to do anything, but the class needs one anyway

        void discoveryStatesMovement()
        {
            if (DistanceRemaining == 0)
            {
                double newAngle = rand.Next(-90, 90);
                SetTurnRight(newAngle);
                SetAhead(100);
            }
        }
        //moves forward in a semirandom direction

        void fireStateOverhead()
        {
            if (GunTurnRemaining == 0)
            {
                if (shouldFire)
                {
                    Fire(power);
                    shouldFire = false;

                }
            }

            if (Energy < 20) power = lowPower;
            else if (Energy > 40) power = highPower;
            //actual movement for the firing state is relative to our scanned opponent, so it's handled in scan
            //other behavior needed for the while loop is here instead of actual movement
        }

        void sweepScan(ScannedRobotEvent e)
        {
            opponents[e.Name] = e;
            Out.WriteLine(opponents.Count);
        }

        void findScan(ScannedRobotEvent e)
        {
            if (e.Name == targetName)
            {
                SetTurnRadarRightRadians(Utils.NormalRelativeAngle(e.BearingRadians + HeadingRadians - RadarHeadingRadians));
                foundTarget = true;
            }
        }

        void fireScan(ScannedRobotEvent e)
        {
            double angleToEnemy = e.BearingRadians + HeadingRadians;    //converting bearing relative to our heading to an absolute angle

            SetTurnRadarRightRadians(Utils.NormalRelativeAngle(angleToEnemy - RadarHeadingRadians));

            if (e.Energy != opponentEnergy)
            {
                if (opponentEnergy != -1)
                {
                    double dodgeFuzz = rand.Next(-10, 10);
                    SetTurnRight(e.Bearing + 90 + dodgeFuzz);
                    if (DistanceRemaining < 0) SetAhead(100);
                    else SetAhead(-100);
                }
                opponentEnergy = e.Energy;
            }
            //we keep track of our opponent's energy because they expend some when they fire
            //this means we can guess for when our opponent is firing, and can dodge accordingly
            //we dodge by slightly modifying the angle we're moving at and changing direction

            else if (DistanceRemaining == 0)
            {
                if (e.Distance > 250 || e.Distance < 20)
                {
                    SetTurnRight(e.Bearing);
                    SetAhead(100);
                }
                else if (e.Distance < 50)
                {
                    SetTurnRight(e.Bearing);
                    SetAhead(-100);
                }

                else
                {
                    SetTurnRight(e.Bearing + 90);
                    SetAhead(100);
                }

            }

            if (!shouldFire)
            {
                Out.Write("Radar Heading: ");
                Out.WriteLine(Heading - GunHeading + e.Bearing);
                //had to look at the wiki to wrap my head around exactly how to do this
                double enemyX = Math.Sin(angleToEnemy) * e.Distance + X;
                double enemyY = Math.Cos(angleToEnemy) * e.Distance + Y;
                //figure out actual x and y based on their angle and distance
                double bulletVelocity = 20 - (3 * power);

                double bulletDistance = e.Distance;
                double enemyTravelDistance = 0;
                double distanceToNewPosition = 0;
                double newEnemyX = Math.Sin(e.HeadingRadians) * enemyTravelDistance + enemyX;
                double newEnemyY = Math.Cos(e.HeadingRadians) * enemyTravelDistance + enemyY;



                for (int i = 0; i < 40; i++)
                {
                    double bulletTravelTime = Math.Ceiling(bulletDistance / bulletVelocity);    //can't get there part of the way through a turn
                    enemyTravelDistance = e.Velocity * bulletTravelTime; //how far the enemy will have gone when the bullet gets to where they are
                    newEnemyX = Math.Sin(e.HeadingRadians) * enemyTravelDistance + enemyX;
                    newEnemyY = Math.Cos(e.HeadingRadians) * enemyTravelDistance + enemyY;
                    distanceToNewPosition = Math.Sqrt(Math.Pow((newEnemyX - X), 2) + Math.Pow((newEnemyY - Y), 2));  //get the distance to the new position

                    newEnemyX = Math.Min(newEnemyX, BattleFieldWidth);
                    newEnemyX = Math.Max(0, newEnemyX);
                    newEnemyY = Math.Min(newEnemyY, BattleFieldHeight);
                    newEnemyY = Math.Max(0, newEnemyY);
                    //ensure we stay in bounds
                    bulletDistance = distanceToNewPosition;
                    //set vars for next iteration
                }
                //we use trigonometry to guess where our opponent will be based on their current speed and direction
                //we use the travel time of the bullet to determine how far out to exprapolate their position
                //it will take the bullet a different amount of time to get to the new position
                //so then we recalc with the new position, hopefully getting (close enough) to what the actual position needs to be
                double firingAngle = Utils.NormalAbsoluteAngle(Math.Atan2(newEnemyX - X, newEnemyY - Y));
                Out.Write("Adjust Angle By:");
                Out.WriteLine(firingAngle);
                if (firingAngle != double.NaN)
                {
                    double gunAdjustAngle = firingAngle - GunHeadingRadians;
                    SetTurnGunRightRadians(Utils.NormalRelativeAngle(gunAdjustAngle));
                    //SetTurnRadarLeftRadians(gunAdjustAngle);
                }
                //linear targetting, inspired from https://robowiki.net/wiki/Linear_Targeting
                if (GunHeat == 0) shouldFire = true;
                Scan();
            }
        }

        (bool, int) sweepTransition()
        {
            if (RadarTurnRemaining != 0)
            {
                return (false, invalid);
            }
            //if we're still turning, don't transition (so state int doesn't matter)

            else if (opponents.Count == 0)
            {
                return (true, sweep);
            }
            //we didn't find an opponent after completing our rotation
            //transition back into sweep and go again

            else
            {
                double minDistance = 1000000; //initialize to something large so it'll get replaced
                foreach (var opponent in opponents)
                {
                    ScannedRobotEvent currentRobot = opponent.Value;
                    if (currentRobot.Distance < minDistance)
                    {
                        minDistance = currentRobot.Distance;
                        targetName = currentRobot.Name;
                    }
                }
                //find the robot closest to us (when we scanned, anyway. good enough)
                if (opponents.Count == 1)
                {
                    foundOneOpponent += 1;
                }
                else
                {
                    foundOneOpponent = 0;
                }
                //keeps track of if we've found multiple opponents
                //simplifies behavior to be more effective in a 1v1

                opponents.Clear(); //reset, since these values will be old now
                return (true, find);
            }
        }

        (bool, int) findTransition()
        {
            if (foundTarget)
            {
                return (true, fire);
            }

            else
            {
                if (RadarTurnRemaining == 0)
                {
                    return (true, sweep);
                }
                //completed rotation without finding the target
                //go back to sweeping
                else
                {
                    return (false, invalid);
                }
                //still looking. stay where we are
            }
        }

        (bool, int) fireTransition()
        {
            if (Time - latestScanTurn > 5)
            {
                return (true, sweep);
            }
            //if we lose track of our opponent, go back to scanning for opponents
            return (false, invalid);
            //if we're still locked on, stay locked on
        }

        public override void Run()
        {
            opponents = new Dictionary<string, ScannedRobotEvent>();
            headingList = new List<(double, long)>();

            StateMachine.State sweepField = new StateMachine.State(discoveryStatesInit, discoveryStatesMovement, sweepScan, sweepTransition);

            finiteStateMachine = new StateMachine(sweepField);

            StateMachine.State findOpponent = new StateMachine.State(discoveryStatesInit, discoveryStatesMovement, findScan, findTransition);
            StateMachine.State fightOpponent = new StateMachine.State(fireInit, fireStateOverhead, fireScan, fireTransition);

            finiteStateMachine.addState(findOpponent);
            finiteStateMachine.addState(fightOpponent);

            //sweep is the initial state, so we construct the FSM with that. other two aren't the default, so they're constructed and added later

            BodyColor = Color.Orange;
            GunColor = Color.Orange;
            RadarColor = Color.DarkOrange;
            ScanColor = Color.Orange;

            while (true)
            {
                IsAdjustRadarForGunTurn = true;
                IsAdjustGunForRobotTurn = true;
                IsAdjustRadarForRobotTurn = true;

                finiteStateMachine.movementBehavior();
                finiteStateMachine.transition();
                Execute();
                //all we need to do in our while loop is call the FSM
            }
        }

        public override void OnScannedRobot(ScannedRobotEvent e)
        {
            if (IsTeammate(e.Name))
            {
                return;
            }
            latestScanTurn = e.Time;
            finiteStateMachine.scanBehavior(e);

        }

        public override void OnHitWall(HitWallEvent e)
        {
            SetTurnRight(e.Bearing);
            SetAhead(-200);
        }

        public override void OnBulletHit(BulletHitEvent e)
        {
            opponentEnergy = e.VictimEnergy;
        }

        public override void OnHitRobot(HitRobotEvent e)
        {
            if (IsTeammate(e.Name))
            {
                SetTurnRight(e.Bearing);
                SetAhead(-100);
            }
            else
            {
                SetTurnRight(e.Bearing);
                double angleToEnemy = e.BearingRadians + HeadingRadians;    //converting bearing relative to our heading to an absolute angle
                SetTurnRadarRightRadians(Utils.NormalRelativeAngle(angleToEnemy - RadarHeadingRadians));
                SetTurnGunRightRadians(Utils.NormalRelativeAngle(angleToEnemy - GunHeadingRadians));
                if (GunHeat == 0) Fire(3);

                if (Energy > e.Energy)
                {
                    SetAhead(100);
                }
                else
                {
                    SetAhead(-100);
                }
                opponentEnergy = e.Energy;
            }
        }


    }
}

