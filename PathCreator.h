
enum PathCreatorState
{
	PathCreator_None,
	PathCreator_CreatingPath,
	PathCreator_PlacingUnits, // or something
};

struct PathCreator
{
	u32 hotUnit;
	PathCreatorState state;

	Rectangle2D resetUnitButton;
	Rectangle2D	finishButton;
};

static PathCreator InitPathCreator()
{
	PathCreator ret;
	ret.hotUnit = 0xFFFFFFFF;
	ret.state = PathCreator_None;
	ret.finishButton = CreateRectangle2D(V2(0.8f, 0.7f), 0.1f, 0.1f);
	ret.resetUnitButton = CreateRectangle2D(V2(0.8f, 0.8f), 0.1f, 0.1f);

	return ret;
}

static void AddInstruction(UnitInstructionArray *p, UnitInstruction i)
{
	if (p->amount + 1 < MaxUnitInstructions)
	{
		(*p)[p->amount++] = i;
	}
	else
	{
		Die;
	}

}

static v3i GetPhysicalPositionAfterMove(Entity *e)
{
	if ((e->temporaryFlags & EntityFlag_IsMoving))
	{
		return e->interpolation.dir + e->physicalPos;
	}

	return e->physicalPos;
}
