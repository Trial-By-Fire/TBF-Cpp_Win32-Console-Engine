// Parent Header
#include "Game_Util.hpp"




namespace Game
{
	// Functions

	// Public

	// Character

	bool Character::AtFinish(Level& _level)
	{
		Vector2D collisionPostion = Position;

		collisionPostion.Y -= 0.085f;

		sInt cellCollided = _level.GetCellAtPosition(collisionPostion);

		return cellCollided == ELevelCell::Finish;
	}

	bool Character::IsGrounded(Level& _level)
	{
		Vector2D collisionPostion = Position;

		collisionPostion.Y -= 0.085f;

		sInt cellCollided = _level.GetCellAtPosition(collisionPostion);

		return cellCollided == ELevelCell::Ground || cellCollided == ELevelCell::Finish;
	}

	void Character::Load()
	{
		Sprite =
		{
			0,
			BACKGROUND_INTENSITY | BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE
		};

		Position = { -0.975f, 0.075f };

		VerticalVelocity = 0.0f;

		ShouldJump = false,
		Fell       = false;

		Active_MoveState = ECharacter_Move::Dont;
		MoveState        = ECharacter_Move::Dont;
	}

	void Character::Update(Level& _level)
	{
		if (Fell == true) return;


		ro float32 deltaTime = SCast<ro float32>(Timing::GetContext().DeltaTime.count());

		unbound float32 velocity = 1.0f;
		unbound float32 gravity  = 0.00004f;

		Vector2D collisionPostion = Position;

		collisionPostion.Y -= 0.085f;

		sInt cellCollided = _level.GetCellAtPosition(collisionPostion);

		if (cellCollided == ELevelCell::Ground || cellCollided == ELevelCell::Finish)
		{
			VerticalVelocity = 0.0f;

			Position.Y = -0.9f;

			Active_MoveState = MoveState;
		}
		else if (cellCollided == ELevelCell::Pit)
		{
			VerticalVelocity = 0.0f;

			Position.Y = -0.9f;

			Fell = true;
		}
		else
		{
			VerticalVelocity -= gravity * deltaTime;

			Position.Y += VerticalVelocity;
		}

		if (cellCollided == ELevelCell::Finish) return;

		if (ShouldJump && cellCollided == ELevelCell::Ground)
		{
			WriteToLog(L"Giving character jump velocity");

			VerticalVelocity = 0.075f * deltaTime;

			Position.Y = -0.75f;

			ShouldJump = false;
		}

		switch (Active_MoveState)
		{
			case ECharacter_Move::Left:
			{
				if (Position.X > -1.0f)
				{
					Position.X -= velocity * deltaTime;
				}

				break;
			}
			case ECharacter_Move::Right:
			{
				if (Position.X < 1.0f)
				{
					Position.X += velocity * deltaTime;
				}

				break;
			}
		}
	}

	void Character::Render()
	{
		if (Fell) return;

		COORD renderCoord = Convert_Vector2D_ToRenderCoord(Position);

		WriteToPersistentSection(1, WString(L"Pos X: ") + ToWString(Position.X));
		WriteToPersistentSection(3, WString(L"Pos Y: ") + ToWString(Position.Y));

		WriteToPersistentSection(2, WString(L"RC  X: ") + ToWString(renderCoord.X));
		WriteToPersistentSection(4, WString(L"RC  Y: ") + ToWString(renderCoord.Y));

		if (renderCoord.Y < 0                ) renderCoord.Y = 0;
		if (renderCoord.Y > Renderer::GameEnd) renderCoord.Y = Renderer::GameEnd;

		WriteToBufferCells(getPtr(Sprite), renderCoord, renderCoord);

		if CompileTime (DebugEnabled)
		{
			unbound Cell colliderView = 
			{
				0,
				BACKGROUND_INTENSITY | BACKGROUND_RED | BACKGROUND_GREEN
			};

			COORD colliderViewCoord;

			Vector2D collisionPostion = Position;

			collisionPostion.Y -= 0.085f;

			colliderViewCoord = Convert_Vector2D_ToRenderCoord(collisionPostion);

			if (colliderViewCoord.Y < 0                ) colliderViewCoord.Y = 0;
			if (colliderViewCoord.Y > Renderer::GameEnd) colliderViewCoord.Y = Renderer::GameEnd;

			WriteToBufferCells(getPtr(colliderView), colliderViewCoord, colliderViewCoord);
		}
	}

	// Level

	sInt Level::GetCellAtPosition(Vector2D _position)
	{
		COORD renderCoord = Convert_Vector2D_ToRenderCoord(_position);

		ptr<Cell> cellBuffer = RCast<Cell>(this);

		uIntDM lineOffset = renderCoord.Y * Renderer::BufferWidth;
		uIntDM colOffset  = renderCoord.X;

		uIntDM totalOffset = lineOffset + colOffset;

		return cellBuffer[totalOffset].Attributes;
	}

	void Level::SetCells(COORD _firstCell, COORD _lastCell, ELevelCell _cellType) 
	{
		uIntDM lineOffset = _firstCell.Y * Renderer::BufferWidth;
		uIntDM colOffset  = _firstCell.X;

		uIntDM totalOffset = lineOffset + colOffset;

		ptr<Cell> levelCellBuffer = RCast<Cell>(this);

		ptr<Cell> bufferOffset = getPtr(levelCellBuffer[totalOffset]);

		uIntDM dataSize = totalOffset;

		lineOffset = _lastCell.Y * Renderer::BufferWidth;

		colOffset  = _lastCell.X;

		totalOffset = lineOffset + colOffset;

		dataSize = totalOffset - dataSize;

		if (dataSize == 0) dataSize = 1;

		unbound DynamicArray<Cell> setCellBuffer;
		
		setCellBuffer.resize(dataSize);

		for (uIntDM index = 0; index < dataSize; index++)
		{
			setCellBuffer.at(index).Char.UnicodeChar = 0;
			setCellBuffer.at(index).Attributes       = (WORD)_cellType;
		}

		Memory::FormatWithData(bufferOffset, setCellBuffer.data(), dataSize);
	}

	void Level::Render()
	{
		COORD 
			screenStart = {                     0,                 0 }, 
			screenEnd   = { Renderer::BufferWidth, Renderer::GameEnd } ;

		WriteToBufferCells(RCast<Cell>(this), screenStart, screenEnd);
	}

	// Space

	COORD Convert_Vector2D_ToRenderCoord(Vector2D _vector)
	{
		unbound float32 
			offsetX = SCast<float32>(Renderer::BufferWidth) / 2.0f, 
			offsetY = SCast<float32>(Renderer::GameEnd    ) / 2.0f;

		float32 
			convertedX = _vector.X * (SCast<float32>(Renderer::BufferWidth) / 2.0f), 
			convertedY = _vector.Y * (SCast<float32>(Renderer::GameEnd    ) / 2.0f);

		COORD renderingCoord;

		renderingCoord.X = SCast<sInt16>(convertedX + offsetX   );	
		renderingCoord.Y = SCast<sInt16>(offsetY    - convertedY);

		if (renderingCoord.X >= Renderer::BufferWidth) renderingCoord.X = Renderer::BufferWidth - 1;

		return renderingCoord;
	}


	// General Rendering

	void ChangeCellsTo_Grey(ptr<Cell> _renderCells, uIntDM _length)
	{
		for (uIntDM cellIndex = 0; cellIndex < _length; cellIndex++)
		{
			_renderCells[cellIndex].Attributes = FOREGROUND_INTENSITY;
		}
	}

	void ChangeCellsTo_White(ptr<Cell> _renderCells, uIntDM _length)
	{
		for (uIntDM cellIndex = 0; cellIndex < _length; cellIndex++)
		{
			_renderCells[cellIndex].Attributes = Console_WhiteCell;
		}
	}


	// UI

	// Text

	void UI_Text::Create 
	(
		WString _content, 
		COORD   _startingCell, 
		COORD   _endingCell,
		bool    _shouldCenter
	)
	{
		Content = move(_content);

		RenderCells = Memory::GlobalAllocate<Cell>(Content.size() + 1);

		for (uIntDM cellIndex = 0; cellIndex < Content.size(); cellIndex++)
		{
			RenderCells[cellIndex].Char.UnicodeChar = Content[cellIndex];
			RenderCells[cellIndex].Attributes       = Console_WhiteCell;
		}

		StartingCell = _startingCell;
		EndingCell   = _endingCell;

		if (_shouldCenter)
		{
			StartingCell.X += (Renderer::BufferWidth / 2) - (SCast<uInt16>(Content.size() + 1) / 2);
			EndingCell  .X += (Renderer::BufferWidth / 2) + (SCast<uInt16>(Content.size() + 1) / 2);

			StartingCell.X--;
			EndingCell  .X--;
		}
		else
		{
			EndingCell.X += SCast<uInt16>(Content.size() + 1);
		}
	}

	void UI_Text::Render()
	{
		WriteToBufferCells(RenderCells, StartingCell, EndingCell);
	}


	// Button

	UI_Button::~UI_Button()
	{
		Text.~UI_Text();
	}

	void UI_Button::Create 
	(
		WString        _text, 
		COORD          _startingCell, 
		COORD          _endingCell, 
		bool           _shouldCenter,
		Void_Function& _callback
	)
	{
		Text.Create(_text, _startingCell, _endingCell, _shouldCenter);

		Callback = getPtr(_callback);
	}

	void UI_Button::Press()
	{
		Callback();
	}

	void UI_Button::Render()
	{
		WriteToBufferCells(Text.RenderCells, Text.StartingCell, Text.EndingCell);
	}


	// Grid

	UI_Grid::~UI_Grid()
	{
		for (uIntDM index = 0; index < Num; index++)
		{
			Buttons[index].~UI_Button();
		}
	}

	void UI_Grid::Add 
	(
		WString        _text, 
		COORD          _startingCell, 
		COORD          _endingCell, 
		bool           _shouldCenter,
		Void_Function& _callback
	)
	{
		if (Num == 0)
		{
			Buttons = Memory::GlobalAllocate<UI_Button>(1);

			Num++;
		}
		else
		{
			ptr<UI_Button> resizeIntermediary = Memory::GlobalReallocate(Buttons, (Num + 1));

			if (resizeIntermediary != NULL)
			{
				Buttons = resizeIntermediary;

				Num++;
			}
			else
			{
				throw RuntimeError("Failed to globally reallocate subscription array.");
			}
		}

		Buttons[Num - 1].Create(_text, _startingCell, _endingCell, _shouldCenter, _callback);

		if (Num != 1)
		{
			ChangeCellsTo_Grey
			(
				Buttons[Num -1].Text.RenderCells, 
				Buttons[Num -1].Text.Content.size()
			);
		}
	}

	void UI_Grid::MoveUp()
	{
		ptr<UI_Text> buttonText = getPtr(Buttons[CurrentIndex].Text);

		if (CurrentIndex > 0)
		{
			ChangeCellsTo_Grey(buttonText->RenderCells, buttonText->Content.size());

			CurrentIndex =  CurrentIndex - 1;

			buttonText = getPtr(Buttons[CurrentIndex].Text);

			ChangeCellsTo_White(buttonText->RenderCells, buttonText->Content.size());
		}
	}

	void UI_Grid::MoveDown()
	{
		ptr<UI_Text> buttonText = getPtr(Buttons[CurrentIndex].Text);

		if (CurrentIndex < (Num - 1))
		{
			ChangeCellsTo_Grey(buttonText->RenderCells, buttonText->Content.size());

			CurrentIndex = CurrentIndex + 1;

			buttonText = getPtr(Buttons[CurrentIndex].Text);

			ChangeCellsTo_White(buttonText->RenderCells, buttonText->Content.size());
		}
	}

	void UI_Grid::Select()
	{
		Buttons[CurrentIndex].Press();
	}

	void UI_Grid::Render()
	{
		for (uIntDM index = 0; index < Num; index++)
		{
			Buttons[index].Render();
		}
	}


	// Widget

	UI_Widget::~UI_Widget()
	{
		for (uIntDM index = 0; index < Num_TextUIs; index++)
		{
			TextUIs[index].~UI_Text();
		}
	}

	void UI_Widget::AddText
	(
		WString _text,
		COORD   _startingCell,
		COORD   _endingCell,
		bool    _shouldCenter
	)
	{
		if (Num_TextUIs == 0)
		{
			TextUIs = Memory::GlobalAllocate<UI_Text>(1);

			Num_TextUIs++;
		}
		else
		{
			ptr<UI_Text> resizeIntermediary = Memory::GlobalReallocate(TextUIs, (Num_TextUIs + 1));

			if (resizeIntermediary != NULL)
			{
				TextUIs = resizeIntermediary;

				Num_TextUIs++;
			}
			else
			{
				throw RuntimeError("Failed to globally reallocate subscription array.");

				exit(1);
			}
		}

		TextUIs[Num_TextUIs - 1].Create
		( 
			_text, 
			_startingCell, 
			_endingCell, 
			_shouldCenter
		);
	}

	void UI_Widget::AddButton 
	(
		WString        _text,
		COORD          _startingCell,
		COORD          _endingCell,
		bool           _shouldCenter,
		Void_Function& _callback
	)
	{
		Grid.Add
		( 
			_text, 
			_startingCell, 
			_endingCell, 
			_shouldCenter, 
			_callback
		);
	}

	void UI_Widget::MoveUp()
	{
		Grid.MoveUp();
	}

	void UI_Widget::MoveDown()
	{
		Grid.MoveDown();
	}

	void UI_Widget::Select()
	{
		Grid.Select();
	}

	void UI_Widget::Render()
	{
		for (uIntDM index = 0; index < Num_TextUIs; index++)
		{
			TextUIs[index].Render();
		}

		Grid.Render();
	}
}
