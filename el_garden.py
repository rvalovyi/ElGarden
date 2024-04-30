import logging
from _testinternalcapi import get_config

from config import *
from content_ua import *
import time
from typing import Any, Dict
import asyncio

from aiogram import Bot, Dispatcher, F, Router
from aiogram.utils.formatting import Text, Bold, as_list, as_marked_section, as_key_value
from aiogram.fsm.context import FSMContext
from aiogram.fsm.state import State, StatesGroup
from aiogram.enums import ParseMode
from aiogram.filters import Command, CommandStart
from aiogram.types import (
    KeyboardButton,
    Message,
    ReplyKeyboardMarkup,
    ReplyKeyboardRemove,
)
from aiogram.utils.keyboard import ReplyKeyboardBuilder
from client import *

form_router = Router()

# States
class Form(StatesGroup):
    main = State()
    select_sys_switch = State()
    check_setting = State()
    change_setting = State()
    entered_light_on = State()
    entered_light_off = State()
    confirm_light = State()
    entered_pump_on = State()
    entered_pump_off = State()
    entered_pump_night = State()
    confirm_pump = State()

@form_router.message(CommandStart())
async def command_start(message: Message, state: FSMContext) -> None:
    if authentication(message=message) == False:
        await message.answer(
            SYSTEM_UNKNOWN_USER,
            reply_markup=ReplyKeyboardRemove(),
        )
        return
    sys_state = get_state()
    config = get_config()
    if sys_state != None and config != None:
        await show_state(message=message, sys_state=sys_state, config=config)
        await state.set_state(Form.main)
        value_system_running = sys_state["is_running"]
        await message.answer(
            f"{PROPOSE_MAIN}",
            reply_markup=ReplyKeyboardMarkup(
                keyboard=[
                    [
                        KeyboardButton(text=f"{KEY_MAIN_SW_ON}" if value_system_running == False else f"{KEY_MAIN_SW_OFF}"),
                        KeyboardButton(text=f"{KEY_MAIN_SETTINGS}"),
                    ]
                ],
                resize_keyboard=True,
            ),
        )
    else:
        await message.answer(
            SYSTEM_FAILURE,
            reply_markup=ReplyKeyboardRemove(),
        )


#=== Cancel =============================================================================
@form_router.message(Command("cancel"))
@form_router.message(F.text.casefold() == "cancel")
async def cancel_handler(message: Message, state: FSMContext) -> None:
    """
    Allow user to cancel any action
    """
    current_state = await state.get_state()
    if current_state is None:
        return

    logging.info("Cancelling state %r", current_state)
    await message.answer(
        WARNING_CANCELLED,
        reply_markup=ReplyKeyboardRemove(),
    )
    await state.clear()
    await command_start(message=message, state=state)

#=== Settings =============================================================================
@form_router.message(Form.main, F.text.casefold() == KEY_MAIN_SETTINGS.lower())
async def process_settings(message: Message, state: FSMContext) -> None:
    config = get_config()
    if config != None:
        await show_settings(message=message, config=config)
        builder = ReplyKeyboardBuilder()
        builder.row(
            KeyboardButton(text=KEY_OPTION_LIGHT),
            KeyboardButton(text=KEY_OPTION_PUMP)
        )
        builder.row(
            KeyboardButton(text=KEY_OPTION_PH),
            KeyboardButton(text=KEY_OPTION_EC)
        )
        builder.row(
            KeyboardButton(text=KEY_OPTION_BACK)
        )
        await message.answer(
            f"{PROPOSE_SETTINGS}",
            reply_markup=builder.as_markup(resize_keyboard=True),
        )
        await state.set_state(Form.check_setting)
    else:
        await message.answer(
            SYSTEM_FAILURE,
            reply_markup=ReplyKeyboardRemove(),
        )
        await state.clear()
        await command_start(message=message, state=state)


#=== System switching =============================================================================
@form_router.message(Form.main, F.text.casefold() == KEY_MAIN_SW_OFF.lower())
@form_router.message(Form.main, F.text.casefold() == KEY_MAIN_SW_ON.lower())
async def confirm_sys_sw(message: Message, state: FSMContext) -> None:
    cmd = message.text
    sys_state = get_state()
    config = get_config()
    if config != None and sys_state != None:
        value_system_running = sys_state["is_running"]
        if cmd == KEY_MAIN_SW_OFF and value_system_running == False:
            await exit_confirm_sys_sw(message=message, state=state, answer=MESSAGE_SYS_ALREADY_OFF)
        elif cmd == KEY_MAIN_SW_ON and value_system_running == True:
            await exit_confirm_sys_sw(message=message, state=state, answer=MESSAGE_SYS_ALREADY_ON)
        else:
            await state.set_state(Form.select_sys_switch)
            await state.update_data(key_main_sw=str(cmd))
            await message.answer(
                ASK_SYS_SW_OFF if cmd == KEY_MAIN_SW_OFF else ASK_SYS_SW_ON,
                reply_markup=ReplyKeyboardMarkup(
                    keyboard=[
                        [
                            KeyboardButton(text=f"{CONFIRM_YES}"),
                            KeyboardButton(text=f"{CONFIRM_NO}"),
                        ]
                    ],
                    resize_keyboard=True,
                ),
            )
    else:
        await message.answer(
            SYSTEM_FAILURE,
            reply_markup=ReplyKeyboardRemove(),
        )
        await state.clear()
        await command_start(message=message, state=state)


async def exit_confirm_sys_sw(message: Message, state: FSMContext, answer=None) -> None:
    if answer != None:
        await message.answer(
            answer,
            reply_markup=ReplyKeyboardRemove(),
        )
    else:
        await message.answer(
            SYSTEM_FAILURE,
            reply_markup=ReplyKeyboardRemove(),
        )
    await state.set_state(Form.main)
    await state.clear()
    await command_start(message=message, state=state)


@form_router.message(Form.select_sys_switch, F.text.casefold() == CONFIRM_YES.lower())
async def process_sys_sw(message: Message, state: FSMContext) -> None:
    data = await state.get_data()
    key_main_sw = data["key_main_sw"]
    if key_main_sw != None:
        if key_main_sw == KEY_MAIN_SW_ON:
            res = set_config({'running': True})
        else:
            res = set_config({'running': False})
        if res == 0:
            await message.answer(
                STATE_SYSTEM_ON if key_main_sw == KEY_MAIN_SW_ON else STATE_SYSTEM_OFF,
                reply_markup=ReplyKeyboardRemove(),
            )
            if key_main_sw == KEY_MAIN_SW_ON:
                time.sleep(1.5)
        else:
            await message.answer(
                SYSTEM_FAILURE,
                reply_markup=ReplyKeyboardRemove(),
            )
    else:
        await message.answer(
            SYSTEM_FAILURE,
            reply_markup=ReplyKeyboardRemove(),
        )
    await state.set_state(Form.main)
    await state.clear()
    await command_start(message=message, state=state)


@form_router.message(Form.select_sys_switch)
async def exit_settings(message: Message, state: FSMContext) -> None:
    await state.set_state(Form.main)
    await state.clear()
    await command_start(message=message, state=state)


#=== Settings key prociessing ================================================================================
#-------------------------------------------------------------------------------------------- KEY_OPTION_LIGHT
@form_router.message(Form.check_setting, F.text.casefold() == KEY_OPTION_LIGHT.lower())
async def enter_light_on(message: Message, state: FSMContext) -> None:
    builder = ReplyKeyboardBuilder()
    for i in range(0, 24):
        builder.add(KeyboardButton(text=str(i)))
    builder.adjust(6)

    await message.answer(
        f"{PROPOSE_LIGHT_ON}",
        reply_markup=builder.as_markup(resize_keyboard=True),
    )

    await state.set_state(Form.entered_light_on)


#-------------------------------------------------------------------------------------------- KEY_OPTION_PUMP
@form_router.message(Form.check_setting, F.text.casefold() == KEY_OPTION_PUMP.lower())
async def enter_pump_on(message: Message, state: FSMContext) -> None:
    builder = ReplyKeyboardBuilder()
    for i in range(5, 65, 5):
        builder.add(KeyboardButton(text=str(i)))
    builder.adjust(6)

    await message.answer(
        f"{PROPOSE_PUMP_ON}",
        reply_markup=builder.as_markup(resize_keyboard=True),
    )

    await state.set_state(Form.entered_pump_on)


#-------------------------------------------------------------------------------------------- KEY_OPTION_BACK
@form_router.message(Form.check_setting, F.text.casefold() == KEY_OPTION_BACK.lower())
async def back_from_settimgs(message: Message, state: FSMContext) -> None:
    await state.set_state(Form.main)
    await state.clear()
    await command_start(message=message, state=state)


#=== Settings Light prociessing ================================================================================
@form_router.message(Form.entered_light_on)
async def process_light_off(message: Message, state: FSMContext, light_on=-1) -> None:
    if light_on == -1:
        light_on = int(message.text)

    if light_on >= 0 and light_on <= 23:
        await state.update_data(light_on=str(light_on))
        builder = ReplyKeyboardBuilder()
        for i in range(0, 24):
            builder.add(KeyboardButton(text=str(i)))
        builder.adjust(6)
        await message.answer(
            f"{PROPOSE_LIGHT_OFF}",
            reply_markup=builder.as_markup(resize_keyboard=True),
        )
        await state.set_state(Form.entered_light_off)
    else:
        await message.answer(
            f"{WARNING_INCORRECT_DATA}",
            reply_markup=ReplyKeyboardRemove(),
        )
        await state.set_state(Form.check_setting)
        await enter_light_on(message=message, state=state)


@form_router.message(Form.entered_light_off)
async def confirm_light(message: Message, state: FSMContext) -> None:
    light_off = int(message.text)
    if light_off >= 0 and light_off <= 23:
        data = await state.update_data(light_off=str(light_off))
        light_on = int(data["light_on"])
        content = as_list(
            as_marked_section(
                Text(f"{TITLE_ENTERED}:"),
                as_key_value(f"{TITLE_LIGHT_ON}", f"{light_on}:00"),
                as_key_value(f"{TITLE_LIGHT_OFF}", f"{light_off}:00"),
                Text(f"{ASK_CONFIRMATION}:"),
                marker="  ",
            ),
            sep="\n\n",
        )
        await message.answer(
            **content.as_kwargs(),
            reply_markup=ReplyKeyboardMarkup(
                keyboard=[
                    [
                        KeyboardButton(text=f"{CONFIRM_YES}"),
                        KeyboardButton(text=f"{CONFIRM_NO}"),
                    ]
                ],
                resize_keyboard=True,
            ),
        )
        await state.set_state(Form.confirm_light)
    else:
        await message.answer(
            f"{WARNING_INCORRECT_DATA}",
            reply_markup=ReplyKeyboardRemove(),
        )
        await state.set_state(Form.entered_light_on)
        data = await state.get_data()
        await process_light_off(message=message, state=state, light_on=int(data["light_on"]))


#@form_router.message(Form.confirm_light)
#async def process_unknown_write_bots(message: Message, state: FSMContext) -> None:
#    await message.reply(WARNING_INCORRECT_DATA)
#    await state.set_state(Form.check_setting)
#    await process_settings(message=message, state=state)


@form_router.message(Form.confirm_light, F.text.casefold() == CONFIRM_NO.lower())
async def process_unknown_write_bots(message: Message, state: FSMContext) -> None:
    await message.reply(WARNING_CONFIG_CANCELLED)
    await state.set_state(Form.check_setting)
    await process_settings(message=message, state=state)


@form_router.message(Form.confirm_light, F.text.casefold() == CONFIRM_YES.lower())
async def process_confirm_light(message: Message, state: FSMContext) -> None:
    config = get_config()
    data = await state.get_data()
    config["light_on"] = int(data["light_on"])
    config["light_off"] = int(data["light_off"])
    res = set_config(config)
    if res == 0:
        await message.answer(
            MESSAGE_CONFIG_CHANGED,
            reply_markup=ReplyKeyboardRemove(),
        )
        # config = get_config()
        # if config != None:
        #     await show_settings(message=message, config=config)
    else:
        await message.answer(
            WARNING_CONFIG_CHANGED,
            reply_markup=ReplyKeyboardRemove(),
        )
    await state.set_state(Form.check_setting)
    await process_settings(message=message, state=state)


#=== Settings Pump prociessing ================================================================================
@form_router.message(Form.entered_pump_on)
async def enter_pump_off(message: Message, state: FSMContext, pump_on=-1) -> None:
    if pump_on == -1:
        pump_on = int(message.text)

    if pump_on >= 5 and pump_on <= 60:
        await state.update_data(pump_on=str(pump_on))
        builder = ReplyKeyboardBuilder()
        for i in range(5, 65, 5):
            builder.add(KeyboardButton(text=str(i)))
        builder.adjust(6)
        await message.answer(
            f"{PROPOSE_PUMP_OFF}",
            reply_markup=builder.as_markup(resize_keyboard=True),
        )
        await state.set_state(Form.entered_pump_off)
    else:
        await message.answer(
            f"{WARNING_INCORRECT_DATA}",
            reply_markup=ReplyKeyboardRemove(),
        )
        await state.set_state(Form.check_setting)
        await enter_pump_on(message=message, state=state)


@form_router.message(Form.entered_pump_off)
async def enter_pump_night(message: Message, state: FSMContext, pump_off=-1) -> None:
    if pump_off == -1:
        pump_off = int(message.text)

    if pump_off >= 5 and pump_off <= 60:
        data = await state.update_data(pump_off=str(pump_off))
        # pump_on = int(data["pump_on"])
        builder = ReplyKeyboardBuilder()
        for i in range(1, 25):
            builder.add(KeyboardButton(text=str(i)))
        builder.adjust(6)
        await message.answer(
            f"{PROPOSE_PUMP_NIGHT}",
            reply_markup=builder.as_markup(resize_keyboard=True),
        )
        await state.set_state(Form.entered_pump_night)
    else:
        await message.answer(
            f"{WARNING_INCORRECT_DATA}",
            reply_markup=ReplyKeyboardRemove(),
        )
        await state.set_state(Form.entered_pump_on)
        data = await state.get_data()
        await enter_pump_off(message=message, state=state, pump_on=int(data["pump_on"]))


@form_router.message(Form.entered_pump_night)
async def confirm_pump(message: Message, state: FSMContext) -> None:
    pump_night = int(message.text)
    if pump_night >= 1 and pump_night <= 24:
        data = await state.update_data(pump_night=str(pump_night))
        pump_on = int(data["pump_on"])
        pump_off = int(data["pump_off"])
        pump_night = int(data["pump_night"])
        content = as_list(
            as_marked_section(
                Text(f"{TITLE_ENTERED}:"),
                as_key_value(f"{TITLE_PUMP_ON}", f"{pump_on} {MESSAGE_MINUTES}"),
                as_key_value(f"{TITLE_PUMP_OFF}", f"{pump_off} {MESSAGE_MINUTES}"),
                as_key_value(f"{TITLE_PUMP_NIGHT}", f"{pump_night} {MESSAGE_TIMES}"),
                Text(f"{ASK_CONFIRMATION}:"),
                marker="  ",
            ),
            sep="\n\n",
        )
        await message.answer(
            **content.as_kwargs(),
            reply_markup=ReplyKeyboardMarkup(
                keyboard=[
                    [
                        KeyboardButton(text=f"{CONFIRM_YES}"),
                        KeyboardButton(text=f"{CONFIRM_NO}"),
                    ]
                ],
                resize_keyboard=True,
            ),
        )
        await state.set_state(Form.confirm_pump)
    else:
        await message.answer(
            f"{WARNING_INCORRECT_DATA}",
            reply_markup=ReplyKeyboardRemove(),
        )
        await state.set_state(Form.entered_pump_off)
        data = await state.get_data()
        await enter_pump_night(message=message, state=state, pump_off=int(data["pump_off"]))


#@form_router.message(Form.confirm_pump)
#async def process_unknown_write_bots(message: Message, state: FSMContext) -> None:
#    await message.reply(WARNING_INCORRECT_DATA)
#    await state.set_state(Form.check_setting)
#    await process_settings(message=message, state=state)


@form_router.message(Form.confirm_pump, F.text.casefold() == CONFIRM_NO.lower())
async def process_unknown_write_bots(message: Message, state: FSMContext) -> None:
    await message.reply(WARNING_CONFIG_CANCELLED)
    await state.set_state(Form.check_setting)
    await process_settings(message=message, state=state)


@form_router.message(Form.confirm_pump, F.text.casefold() == CONFIRM_YES.lower())
async def process_confirm_pump(message: Message, state: FSMContext) -> None:
    config = get_config()
    data = await state.get_data()
    config["pump_on"] = int(data["pump_on"])
    config["pump_off"] = int(data["pump_off"])
    config["pump_night"] = int(data["pump_night"])
    res = set_config(config)
    if res == 0:
        await message.answer(
            MESSAGE_CONFIG_CHANGED,
            reply_markup=ReplyKeyboardRemove(),
        )
        # config = get_config()
        # if config != None:
        #     await show_settings(message=message, config=config)
    else:
        await message.answer(
            WARNING_CONFIG_CHANGED,
            reply_markup=ReplyKeyboardRemove(),
        )
    await state.set_state(Form.check_setting)
    await process_settings(message=message, state=state)


#=== SHOW STATE ======================================================================================
async def show_state(message: Message, sys_state=None, config=None) -> None:
    if sys_state != None and config!=None:
        # "is_running": true,
        # "is_light_on": true,
        # "is_pump_on": false,
        # "pH": 468,
        # "EC": 1784,
        # "date": "28.04.2024",
        # "time": "18:59:49",
        # "timer_chg_light": "21:00:00",
        # "timer_chg_pump": "19:00:00"
        value_system_running = sys_state["is_running"]
        value_light = STATE_ON if sys_state["is_light_on"] == True else STATE_OFF
        value_pump = STATE_ON if sys_state["is_pump_on"] == True else STATE_OFF
        value_pH = sys_state["pH"]
        value_EC = sys_state["EC"]
        value_date = sys_state["date"]
        value_time = sys_state["time"]
        value_timer_chg_light = sys_state["timer_chg_light"]
        value_timer_chg_pump = sys_state["timer_chg_pump"]
        value_norm_pH = config["pH"]
        value_norm_EC = config["EC"]

#        as_key_value(f"{TITLE_SYSTEM_STATE}", f"{value_system_running}"),
        content = as_list(
            as_marked_section(
                Bold(f"{TITLE_SYSTEM_STATE}:"),
                as_key_value(f"{TITLE_DATE}", f"{value_date}"),
                as_key_value(f"{TITLE_TIME}", f"{value_time}"),
                Text(f" "),
                as_key_value(f"{TITLE_LIGHT}", f"{value_light}"),
                as_key_value(f"{TITLE_NEXT_LIGHT_ON}" if value_light == STATE_OFF else f"{TITLE_NEXT_LIGHT_OFF}", f"{value_timer_chg_light}"),
                Text(f" "),
                as_key_value(f"{TITLE_PUMP}", f"{value_pump}"),
                as_key_value(f"{TITLE_NEXT_PUMP_ON}" if value_pump == STATE_OFF else f"{TITLE_NEXT_PUMP_OFF}", f"{value_timer_chg_pump}"),
                Text(f" "),
                as_key_value(f"{TITLE_PH}", f"{value_pH} / {TITLE_NORM}: {value_norm_pH}"),
                as_key_value(f"{TITLE_EC}", f"{value_EC} / {TITLE_NORM}: {value_norm_EC}"),
                marker="  ",
            ),
            sep="\n\n",
        ) if value_system_running == True else as_list(
           as_marked_section(
               Bold(f"{TITLE_SYSTEM_STATE}:"),
               Text(f"{STATE_SYSTEM_OFF}"),
               marker="  ",
           ),
           sep="\n\n",
        )
        await message.answer(
            **content.as_kwargs(),
            reply_markup=ReplyKeyboardRemove(),
        )


async def show_settings(message: Message, config=None) -> None:
    if config != None:
        value_light_on = config["light_on"]
        value_light_off = config["light_off"]
        value_pump_on = config["pump_on"]
        value_pump_off = config["pump_off"]
        value_pump_night = config["pump_night"]
        content = as_list(
            as_marked_section(
                Bold(f"{TITLE_SYSTEM_SETTINGS}:"),
                as_key_value(f"{TITLE_LIGHT_ON}", f"{value_light_on}:00"),
                as_key_value(f"{TITLE_LIGHT_OFF}", f"{value_light_off}:00"),
                as_key_value(f"{TITLE_PUMP_ON}", f"{value_pump_on} {MESSAGE_MINUTES}"),
                as_key_value(f"{TITLE_PUMP_OFF}", f"{value_pump_off} {MESSAGE_MINUTES}"),
                as_key_value(f"{TITLE_PUMP_NIGHT}", value_pump_night),
                marker="  ",
            ),
            sep="\n\n",
        )
        await message.answer(
            **content.as_kwargs(),
            reply_markup=ReplyKeyboardRemove(),
        )


#@form_router.message(Form.confirm_light)
#async def process_cancel_light(message: Message, state: FSMContext) -> None:
#    await message.answer(
#        WARNING_CONFIG_CANCELLED,
#        reply_markup=ReplyKeyboardRemove(),
#    )
#    await state.clear()
#    await command_start(message=message, state=state)


async def main():
    filepath = Path(FILE_CFG)
    if Path(filepath).exists():
        with open(FILE_CFG, 'r') as f:
            config = json.load(f)
            if "token" in config:
                bot = Bot(token=config["token"], parse_mode=ParseMode.HTML)
                dp = Dispatcher()
                dp.include_router(form_router)
                await dp.start_polling(bot)
            else:
                print(f'The token is not in the telegram configuration.')
                print(f'You should use config.py to config telegram bot.')
                exit(-1)
    else:
        print(f'The telegram configuration is absent.')
        print(f'You should use config.py to config telegram bot.')
        exit(-1)



if __name__ == "__main__":
    logging.basicConfig(level=logging.INFO, stream=sys.stdout)
    asyncio.run(main())
