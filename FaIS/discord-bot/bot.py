import os
import random
import copy

import discord
from discord.ext import commands
from dotenv import load_dotenv

from utils import *

load_dotenv()
TOKEN = os.getenv('DISCORD_TOKEN')
GUILD = os.getenv('DISCORD_GUILD')
ADMIN = 'SILVER'

intents = discord.Intents(guilds=True, members=True, messages=True)
bot = commands.Bot(command_prefix='!', intents=intents)

def find_member_by_mention(mention):
    mention_id = copy.copy(mention)
    guild = discord.utils.find(lambda g: g.name == GUILD, bot.guilds)
    if mention_id[:2] == '<@' and mention_id[-1] == '>':
        mention_id = mention_id[2:-1]
        if mention_id[0] == '!':
            mention_id = mention_id[1:]
    member = discord.utils.find(lambda m: m.id == int(mention_id), guild.members)
    return member

@bot.event
async def on_ready():
    guild = discord.utils.get(bot.guilds, name=GUILD)
    print(f'{bot.user} has connected to the guild {guild.name} (id: {guild.id})')

@bot.command(name='info')
async def info(ctx):
    await ctx.send('I am an experimental bot created by SILVER.')

@bot.command(name='insult', help='I am going to hurt your feelings if SILVER says so!')
async def insult(ctx, mention=''):
    if mention == '':
        await ctx.send('You should mention someone.')
        return

    try:
        member = find_member_by_mention(mention)
        if not member:
            raise Exception()
    except:
        await ctx.send('There is no such member. You should put that trash\'s name correctly.')
        return

    if member.name == ADMIN:
        await ctx.send(f'Hahaha... Jokes on you! {ADMIN} has already thought of such edge cases, doofus!')
        return

    insults = ['You are really a piece of shit... You know that, right?!',
            'Stop it! Get some HELP!',
            'You mad bro? Guess what? NOBODY EVER CARES.',
            'Jesus! Your IQ level should be even less than the respect that nobody shows you...']

    await ctx.send('%s, %s' % (mention, random.choice(insults)))

@bot.command(name='respect', help='I am going to respect those who deserve it.')
async def respect(ctx, mention=''):
    if mention == '':
        await ctx.send('You should mention someone.')
        return

    try:
        member = find_member_by_mention(mention)
        if not member:
            raise Exception()
    except:
        await ctx.send('There is no such member. You should put that trash\'s name correctly.')
        return

    if member.name == ADMIN:
        await ctx.send(f'{ADMIN} doesn\'t need your respect but thanks anyways...')
        return

    respects = ['Keep doing whatever you are doing. You are great at it!',
            'Our society needs more people like you!',
            'Good job and well done!',
            'I\'ll spam F for you! F... F... F...']

    await ctx.send('%s, %s' % (mention, random.choice(respects)))

@bot.command(name='what2play', help='I will help you to decide what to play now')
async def what2play(ctx, *argv):
    if len(argv) < 2:
        await ctx.send('Give me more options!')
        return
    await ctx.send(f'My vote is for {random.choice(argv)}!')

@bot.command(name='apologize', help='.!.')
async def apologize(ctx, mention=''):
    if mention == '':
        await ctx.send('You should mention at least two members.')
        return

    try:
        member = find_member_by_mention(mention)
        if not member:
            raise Exception()
    except:
        await ctx.send('There is no such member.')
        return

    if member.name == ADMIN:
        await ctx.send(f'I am created by him, he does not need my apologies...')
        return

    apologies = ['OMG! Did you really think I am going to apologize...',
            'You should be very dumb to think that Shrediddily would actually apologize.',
            'From the bottom of my heart... I... DON\'T HAVE A HEART, YOU SLOW MALFUNCTIONING CREATURE!',
            'Well, I\'m sorry to have such a stupid person like you in our server :( Does that count..?']

    await ctx.send('%s, %s' % (mention, random.choice(apologies)))

@bot.command(name='troll', help='I will troll the shit out of you :P')
async def troll(ctx, mention=''):
    # TODO
    pass

@bot.command(name='funfact', help='I will tell you a fun and maybe fake fact')
async def funfact(ctx, about=''):
    # TODO
    pass

@bot.command(name='test', help='I will test the IQ levels of server members')
async def test(ctx, category=''):
    # TODO
    pass

@bot.command(name='dashboard', help='Show statistics for members')
async def dashboard(ctx, *argv):
    guild = discord.utils.get(bot.guilds, name=GUILD)
    members = guild.members

    if len(argv) > 0:
        members = []
        for m in argv:
            members.append(m)

    df = get_dashboard(members)
    df.head()
    print(df.values)
    # for val in df.values:
    #     dashboard += f'{val[0]}\n'
    
    # await ctx.send(dashboard)


bot.run(TOKEN)
